# C-threads-consumers-producers
This repo contains a c program, which creates multiple threads with consumers and producers.

It takes 5 parameters:
  1st: number of producers.
  2nd: number of consumers.
  3rd: capacity of qeue, which store numbers from producers.
  4rd: how many numbers will produce and store each producer.
  5th: a seed for rand_r function.
  
This programs creates multiple producers and consumers, each one in new thread. Each producer produce a specific number of random numbers and makes a push to a qeue. Each consumer consumes many random numbers and stops when producers have terminated. Each producers and consumer writes its result to the file prod_in.txt and cons_out.txt respectively. Last, each thread ta ascending order prints to stdout its results.
