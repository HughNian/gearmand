/*
  Sample test application.
*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <libgearman/gearman.h>

int main(int argc, char *argvp[])
{
  gearman_st *gear_con;
  gearman_worker_st *worker;

  gearman_job_st *job;
  gearman_result_st *result;
  gearman_result_st *incomming;

  /* Connect to the server or error */
  {
    gearman_return rc;

    gear_con= gearman_create(NULL);

    if (gear_con == NULL)
    {
      fprintf(stderr, "Could not allocation gearman_st\n");
      exit(1);
    }

    rc= gearman_server_add(gear_con, "localhost", 0);

    if (rc != GEARMAN_SUCCESS)
    {
      fprintf(stderr, "Could not add server\n");
      exit(1);
    }
  }

  worker= gearman_worker_create(gear_con, NULL);
  job= gearman_job_create(gear_con, NULL);

  if (worker == NULL)
  {
    fprintf(stderr, "Could not allocate worker\n");
    exit(1);
  }

  if (job == NULL)
  {
    fprintf(stderr, "Could not allocate job\n");
    exit(1);
  }

  incomming= gearman_result_create(gear_con, NULL);
  if (incomming == NULL)
  {
    fprintf(stderr, "Could not incomming buffer\n");
    exit(1);
  }

  result= gearman_result_create(gear_con, NULL);
  if (result == NULL)
  {
    fprintf(stderr, "Could not result buffer\n");
    exit(1);
  }

  /* Break this out */
  {
    gearman_return rc;

    rc= gearman_worker_do(worker, "echo");
    if (rc != GEARMAN_SUCCESS)
    {
      fprintf(stderr, "Could not assign function: %s\n", gearman_strerror(gear_con, rc));
      exit(1);
    }

    /* Just loop and process jobs */
    while (1)
    {
      /* Loop until we take a job */
      do {
        rc= gearman_worker_take(worker, incomming);
      } while (rc != GEARMAN_SUCCESS);

      printf("We got \"%.*s\"(%u) for %s\n", gearman_result_length(incomming), 
             gearman_result_value(incomming), 
             gearman_result_length(incomming), 
             gearman_result_handle(incomming));

      rc= gearman_result_set_handle(result, gearman_result_handle(incomming), gearman_result_handle_length(incomming));
      if (rc != GEARMAN_SUCCESS)
      {
        fprintf(stderr, "Could not set return handle: %s\n", gearman_strerror(gear_con, rc));
        exit(1);
      }

      if (gearman_result_length(incomming))
      {
        size_t x, y;
        char *buffer;
        char *value;

        buffer= calloc(gearman_result_length(incomming), sizeof(char));
        assert(buffer);

        value= gearman_result_value(incomming);
        for (y= 0, x= gearman_result_length(incomming); x; x--, y++)
          buffer[y]= value[x - 1];

        rc= gearman_result_set_value(result, buffer, gearman_result_length(incomming));
        if (rc != GEARMAN_SUCCESS)
        {
          fprintf(stderr, "Could not set return value: %s\n", gearman_strerror(gear_con, rc));
          exit(1);
        }

        free(buffer);
      }
      else
      {
        rc= gearman_result_set_value(result, NULL, 0);
      }

      if (rc != GEARMAN_SUCCESS)
      {
        fprintf(stderr, "Could not set return value: %s\n", gearman_strerror(gear_con, rc));
        exit(1);
      }

      rc= gearman_worker_answer(worker, result);
      printf("Here\n");
      if (rc != GEARMAN_SUCCESS)
      {
        fprintf(stderr, "Not able to return value: %s\n", gearman_strerror(gear_con, rc));
        exit(1);
      }
    }
  }

  return 0;
}