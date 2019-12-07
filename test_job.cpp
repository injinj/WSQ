#include <iostream>
#include "job.h"
#include <thread>
#include <chrono>

using namespace job;

static void
worker_thread_function( JobTaskThread *w ) {
  w->wait_for_termination(); /* run jobs until done */
}

/* cpus used and test iteraitions */
static uint32_t parallel_jobs     = 10000, /* how many parallel jobs */
                num_cores         = 8,    /* can't be more than MAX_TASKS */
                serial_iterations = 1000, /* how many serial test cases */
                task_workload     = 100;  /* the workload to test (iterations)*/

static void
work_task( int &result ) {
  for (uint32_t j = 1; j <= task_workload; j++) {
    /*for (int i = 1; i <= 10; i++) {*/
      result += ( 1 << ( result & 3 )) - j * 2;
      /* use result so it doesn't get optimized away */
      /*asm volatile ( "" : : "g"(&result) : "memory" );*/
    /*}*/
  }
}

static void
work_task_job( JobTaskThread &w,  Job &/*j*/ ) {
  int result = 0;
  /* track job make sure all have run and not twice */
  /* set_job_bit( j.job_id );*/
  /* do the work */
  work_task( result );
  *(int *) w.data += result;
}

static void
start_jobs( JobTaskThread &w,  Job &j,  uint64_t njobs ) {
  Job *jar[ 256 ];
  uint64_t m = 256;
  for ( uint64_t k = 0; k < njobs; k += m ) {
    if ( k + 256 > njobs )
      m = njobs - k;
    for ( uint64_t i = 0; i < m; i++ )
      jar[ i ] = w.create_job_as_child( j, work_task_job );
    w.kick_jobs( jar, m );
  }
}

static void
root_job_function( JobTaskThread &w,  Job &j ) {
  /*set_job_bit( j.job_id );*/
  start_jobs( w, j, parallel_jobs );
}

static const char *
get_arg( int argc, char *argv[], int b, const char *f )
{
  for ( int i = 1; i < argc - b; i++ )
    if ( ::strcmp( f, argv[ i ] ) == 0 ) /* -c cores */
      return argv[ i + b ];
  return nullptr;
}

int serial_total; /* dummy accum for serial tests */
union {
  int total;
  char cache_line[ 64 ];
} par_result[ MAX_TASKS ];

int
main( int argc,  char *argv[] ) {
  const char * graph = get_arg( argc, argv, 0, "-g" ),
             * cores = get_arg( argc, argv, 1, "-c" ),
             * jobs  = get_arg( argc, argv, 1, "-j" ),
             * iters = get_arg( argc, argv, 1, "-i" ),
             * help  = get_arg( argc, argv, 0, "-h" );

  if ( cores != nullptr )
    num_cores = atoi( cores );
  if ( jobs != nullptr )
    parallel_jobs = atoi( jobs );
  if ( iters != nullptr )
    serial_iterations = atoi( iters );
  if ( help != nullptr ||
       num_cores == 0 || parallel_jobs == 0 || serial_iterations == 0 ||
       num_cores >= MAX_TASKS ) {
    printf( "%s [-g] [-c cores] [-j jobs] [-i iters] [-h]\n"
            "   -g       : produce format for graph plotting\n"
            "   -c cores : number of threads to test\n"
            "   -j jobs  : number of jobs t0 run for parallel portion\n"
            "   -i iters : number of iterations to run for serial portion\n",
            argv[ 0 ] );
    printf( "maximum core count is %u\n", MAX_TASKS );
    return 1;
  }

  std::chrono::high_resolution_clock::time_point start_time, end_time;
  uint64_t serial_elapsed_nanos, par_elapsed_nanos, serial_per_job, par_per_job;

  if ( ! graph ) {
    printf( "Sizeof Job Sys Ctx: %lu\n", sizeof( JobSysCtx ) );
    printf( "Sizeof Job Thread:  %lu\n", sizeof( JobTaskThread ) );
    printf( "Sizeof Job:         %lu\n", sizeof( Job ) );
    printf( "Sizeof Job Alloc:   %lu\n", sizeof( JobAllocBlock ) );
    printf( "Number of threads:  %u\n", num_cores );
    printf( "Serial workload:    %u iterations\n", serial_iterations );
    printf( "Parallel workload:  %u jobs\n", parallel_jobs );
  }
  JobSysCtx job_context;
  job_context.activate();

  JobTaskThread * m, /* main thread */
                * w; /* a worker thread */
  /* use start_time as a seed for the worker random */
  m = job_context.initialize_worker( start_time.time_since_epoch().count(),
                                     &par_result[ 0 ] );

  /* start num_cores - 1 threads */
  std::thread worker_threads[ num_cores - 1 ];
  for ( uint32_t i = 1; i < num_cores; i++ ) {
    /* seed the next worker using main rand */
    w = job_context.initialize_worker( m->rand.next(), &par_result[ i ] );
    /* start the worker */
    worker_threads[ i - 1 ] = std::thread( worker_thread_function, w );
  }

  if ( ! graph ) {
    printf( "Workload  Serial Elapsed  Parallel Elapsed  Speedup\n"
            "--------  --------------  ----------------  -------\n" );
  }
  for ( task_workload = 100; task_workload <= 7000; task_workload += 100 ) {
    start_time = std::chrono::high_resolution_clock::now();
    for ( uint32_t j = 0; j < serial_iterations; j++ ) {
      int result = 0;
      work_task( result );
      serial_total += result;
    }
    end_time = std::chrono::high_resolution_clock::now();

    serial_elapsed_nanos =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end_time-start_time)
        .count();
    serial_per_job = serial_elapsed_nanos / serial_iterations;

    /* create the root job, which creates work_tasks */
    start_time = std::chrono::high_resolution_clock::now();
    Job *j = m->create_job( root_job_function );
    m->kick_and_wait_for( *j ); /* wait until all children of job are done */
    j->alloc_block.deref(); /* dereference, not needed anymore */
    end_time = std::chrono::high_resolution_clock::now();

    par_elapsed_nanos =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end_time-start_time)
        .count();
    par_per_job = par_elapsed_nanos / parallel_jobs;

    if ( ! graph ) {
      printf( "%8u  ", task_workload );
      printf( "%11lu ns  ", serial_per_job );
      printf( "%13lu ns  ", par_per_job );
      printf( "%7.2f", (double) serial_per_job / (double) par_per_job );
      if ( serial_per_job < par_per_job )
        printf( "  (- %lu / thr: %lu)",
                par_per_job - serial_per_job,
                ( par_per_job - serial_per_job ) / num_cores );
      printf( "\n" );
    }
    else {
      printf( "%u %lu %lu %.2f\n", task_workload, serial_per_job, par_per_job,
              (double) serial_per_job / (double) par_per_job );
    }
  }
  job_context.deactivate();   /* tell threads to exit */

  /* reap the threads created */
  for ( uint32_t i = 1; i < num_cores; i++ )
    worker_threads[ i - 1 ].join();

  /*check_all_jobs_have_run();
  printf( "All bits set, success!\n" );*/
  return 0;
}

#if 0
std::atomic<uint64_t> bits[ ( parallel_jobs + 1 + 63 ) / 64 ];

static void
set_job_bit( uint64_t job_id ) {
  /* track when jobs have run */
  for (;;) {
    uint64_t o = job_id / 64;
    uint64_t b = bits[ o ].load( std::memory_order_acquire ),
             c = b | ( (uint64_t) 1 << ( job_id % 64 ) );
    assert( b != c ); /* error: Task job_id already run */
    if ( std::atomic_compare_exchange_strong( &bits[ o ], &b, c ) )
      break;
  }
}

static void
check_all_jobs_have_run( void ) {
  /* root job is job_id 0 */
  for ( uint64_t job_id = 0; job_id < parallel_jobs + 1; job_id++ ) {
    uint64_t o = job_id / 64,
             b = bits[ o ].load( std::memory_order_acquire ),
             c = (uint64_t) 1 << ( job_id % 64 );
    assert( ( b & c ) != 0 ); /* error: Task job_id not run!! */
  }
}
#endif
