#include "lib.h"

typedef void (*hi_t)(void); // Create the host function type

int main(void)
{
  void *handle; // Pointer that will store the library handle
  hi_t salue; // function pointer that will retrieve the function address
  char *erreur; // variable that will store the error

  handle = dlopen("./libmathN.so", RTLD_LAZY); // Load the library into memory
  
  if(!handle)
  {
    fprintf(stderr, "dlopen() error: %s\n", dlerror());
    exit(EXIT_FAILURE);
  }

  dlerror(); // Important to check the error return
  salue = (hi_t) dlsym(handle, "saluer"); // Retrieve the function address

  if((erreur = dlerror()) != NULL)
  {
    fprintf(stderr, "dlsym() error (saluer) %s\n", erreur);
    dlclose(handle);
    exit(EXIT_FAILURE);
  }

  if(salue)
  {
    salue(); // Execute the function
  }

  dlclose(handle); // Unload the library from memory

  return EXIT_SUCCESS;
}