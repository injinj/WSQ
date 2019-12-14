# Work Stealing Queue

This is an implementation of a Work Stealing Queue described in a series of
[blog articles](https://blog.molecular-matters.com) by [Stefan
Reinalter](https://twitter.com/molecularmusing) at Molecular Matters, beginning
with [Job System
2.0](https://blog.molecular-matters.com/2015/08/24/job-system-2-0-lock-free-work-stealing-part-1-basics/).

## Origin

In the blog article, Stefan uses a lock-free structure with memory barriers and
compare-exchange primitives.  This algorithm also uses these ideas, but does so
in a slightly different way then described by Stefan.  It dispenses with the
memory barriers and uses compare-exchange with exchange primitives to schedule
jobs using a [Work Stealing
Queue](https://stackoverflow.com/questions/27830691/work-stealing-and-deques)
which is probably taught in many CS classes like [CSE P 506 -
Concurrency](https://courses.cs.washington.edu/courses/csep506/11sp/Home.html).

The version described by Stefan is a variation of a [Chase Lev dequeue](https://www.dre.vanderbilt.edu/~schmidt/PDF/work-stealing-dequeue.pdf).
There is an implementation of that work queue in [Aamanieu](https://github.com/Amanieu)'s
[asyncplusplus](https://github.com/Amanieu/asyncplusplus) in the default task scheduler.
This is the header file for it: [work_steal_queue.h](https://github.com/Amanieu/asyncplusplus/blob/master/src/work_steal_queue.h).

## Description of Algorithm

A Work Stealing Queue is a double ended queue that each thread/core maintains.
The thread which owns the queue puts jobs at the bottom end of the queue and
other threads to steal jobs from the top end, when they have nothing to do in
their own queues.

```console
+--------+ <- entries[ 0 ]
|  top   | <- stealers consume here: job = entries[ top++ ]
|        |
|   ||   |
|        |
|   vv   |
| bottom | <- owner pushes here:    entries[ bottom++ ] = job
|        |    owner consumes here:  job = entries[ --bottom ]
|        |
+--------+ <- entries[ MASK_JOBS ]
```

The parallelism obtained by this lock-free structure is fine grained.  The test
included is able to schedule synthetic jobs with approximately 100 ns (per
thread) of overhead on my <b>i9-7960X</b> cpu.  I surmise that most of the this
is due to the cache coherency overhead of the x86 <b>cmpxchg</b> and
<b>xchg</b> instructions along with push/pop mechanics of the queue.

I measure this 100 ns overhead by putting jobs into the queue with only one
worker thread.

```console
$ git clone https://injinj.github.com/WSQ
$ cd WSQ
$ g++ -Wall -Wextra -std=c++11 -O3 test_job.cpp -pthread
$ a.out -c 1
Sizeof Job Sys Ctx: 528
Sizeof Job Thread:  524408
Sizeof Job:         48
Sizeof Job Alloc:   65480
Number of threads:  1
Serial workload:    1000 iterations
Parallel workload:  10000 jobs
......................................................................
Workload  Serial Elapsed  Parallel Elapsed  Speedup
--------  --------------  ----------------  -------
     100          142 ns            192 ns     0.74  (- 50 / thr: 50)
     200          287 ns            335 ns     0.86  (- 48 / thr: 48)
     300          432 ns            474 ns     0.91  (- 42 / thr: 42)
     400          579 ns            605 ns     0.96  (- 26 / thr: 26)
^C
$ a.out -c 2                                                            
...
Workload  Serial Elapsed  Parallel Elapsed  Speedup
--------  --------------  ----------------  -------
     100          142 ns            106 ns     1.34
     200          289 ns            169 ns     1.71
     300          432 ns            237 ns     1.82
     400          579 ns            308 ns     1.88
^C
$ a.out -c 3
...
Workload  Serial Elapsed  Parallel Elapsed  Speedup
--------  --------------  ----------------  -------
     100          142 ns             87 ns     1.63
     200          287 ns            128 ns     2.24
     300          435 ns            174 ns     2.50
     400          577 ns            218 ns     2.65
^C
$ a.out -c 4
...
Workload  Serial Elapsed  Parallel Elapsed  Speedup
--------  --------------  ----------------  -------
     100          142 ns             70 ns     2.03
     200          289 ns             99 ns     2.92
     300          432 ns            133 ns     3.25
     400          580 ns            174 ns     3.33
^C
```

I also created a gnuplot script to graph the speedup of this synthetic
workload.  This graph is the result of running that.

```console
$ gnuplot
Terminal type set to 'qt'
gnuplot> load "plot.gnuplot"
1-core
2-core
3-core
4-core
5-core
6-core
7-core
8-core
9-core
10-core
11-core
12-core
13-core
14-core
15-core
16-core
```

![Job Stealing Queue](jsq.svg)


