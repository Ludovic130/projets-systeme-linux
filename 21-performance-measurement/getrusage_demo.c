#include "lib.h"

int main(int argc, char *argv[])
{
  int lesquelles;

  struct rusage statistiques; // structure containing the process statistics.

  if(argc == 1)
  {
    lesquelles = RUSAGE_SELF; // information about the calling process itself
  } else {
    system(argv[1]); // Execute the command passed as argument 
    lesquelles = RUSAGE_CHILDREN; // information about terminated child processes
  }

  if(getrusage(lesquelles, &statistiques) != 0) // Get the statistics
  {
    fprintf(stderr, "Unable to get statistics\n");
    exit(EXIT_FAILURE);
  }

  if(getrusage(lesquelles, &statistiques) != 0)
  {
    fprintf(stderr, "Unable to get statistics\n");
    exit(EXIT_FAILURE);
  }

  // Time spent by the process in user mode.
  fprintf(stdout, "Time in user mode %ld s. and %ld ms\n", statistiques.ru_utime.tv_sec, statistiques.ru_utime.tv_usec / 1000);

  // Time spent by the process in kernel mode.
  fprintf(stdout, "Time in kernel mode %ld s. and %ld ms\n", statistiques.ru_stime.tv_sec, statistiques.ru_stime.tv_usec / 1000);

  fprintf(stdout, "\n");
  // Number of minor page faults.
  fprintf(stdout, "Number of minor page faults: %ld\n", statistiques.ru_minflt);
 
  // Number of major page faults.
  fprintf(stdout, "Number of major page faults: %ld\n", statistiques.ru_majflt);

  // Number of times the process was swapped.
  fprintf(stdout, "Number of process swaps: %ld\n", statistiques.ru_nswap);

  return EXIT_SUCCESS;
}