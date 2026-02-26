#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

/*
 * main.c
 *
 * This file is the main file of this project, it aims to collect cpu utilization in 
 * real time to compute an average utilization pourcentage
 *
 * This program suppose you are on Arch Linux distribution or an equivalent distribution with
 * the same filesystem management
 *
 * */


// Structure to store the data of one line of the /proc/stat file where N is the cpu core number
typedef struct {
   unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
   int coreNumber;
} CPUData;

// Create a list of n struct CPUData
CPUData *createCPUData(unsigned int n){
   CPUData *cpu = malloc(sizeof(CPUData)*n);
   
   for(unsigned i = 0; i < n; i++)
      if(!cpu[i]){
         fprintf(stderr, "Error when allocating memory for the CPUData structure\n");
         exit(EXIT_FAILURE);
      }

   return cpu;
}

// Free an existing list of struct CPUData
void destroy_data(CPUData *cpu){
   if(!cpu)
      return NULL;

   free(cpu);
}

// This functions return the cpu core number if it's not the first line of the file, if it is, return -1
int get_cpu_id(char *line){
   if(!line){
      fprintf(stderr, "Invalid arguments passed to the get_cpu_id function");
      return -3;
   }

   if(strncmp(line, "cpu", 3)){
      return -2;
   }
      
   int core = 0;
   int scanned = sscanf(line, "cpu%d", &core);
   if(scanned <= 0){
      return -1;
   }

   return core;
}

// Read one line of the /proc/stat file and store its data to the CPUData struct
void get_cpu_data(CPUData *c, unsigned int limit){
   FILE *f = fopen("/proc/stat", "r");
   if(!f){
      perror("Error while opening the /proc/stat file\n");
      exit(EXIT_FAILURE);
   };
   
   char buff[256];
   int scanned;
   int n;

   while(fgets(buff, sizeof(buff), f)){
      n = get_cpu_id(buff); 
      if(n == -3){
         fprintf(stderr, "Error when parsing the /proc/stat file\n");
         fclose(f);
         return;
      }

      if(n == -2){
         continue;
      }

      if(n >= -1){
         c[n+1].coreNumber = n  
         if(n+1 < limit){
            scanned = sscanf(buff, "%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                     &c[n+1].user, &c[n+1].nice, &c[n+1].system, &c[n+1].idle, &c[n+1].iowait, &c[n+1].irq, &c[n+1].softirq, &c[n+1].steal, &c[n+1].guest, &c[n+1].guest_nice);

            if(scanned != 10){
               fprintf(stderr, "Error when pasring the /proc/stat file (collecting the data)");
               fclose(f);
               exit(EXIT_FAILURE);
            }
         }
         else
            break;
      }
   }

   fclose(f);
   return;
}

double compute_percentage(unsigned long long total_t1, unsigned long long total_t2, unsigned long long idle_t1, unsigned long long idle_t2){

   double usage = 100*(1-(double)(idle_t2 - idle_t1)/(total_t2 - total_t1));
   return usage;
}

int main(){

   CPUData d1, d2;

   while(1){
      read_cpu_data(&d1);
      usleep(500000);
      read_cpu_data(&d2);
      
      unsigned long long total_t1 = d1.user + d1.nice + d1.system + d1.idle + d1.iowait + d1.irq + d1.softirq + d1.steal;
      unsigned long long total_t2 = d2.user + d2.nice + d2.system + d2.idle + d2.iowait + d2.irq + d2.softirq + d2.steal;

      unsigned long long real_idle_t1 = d1.idle + d1.iowait;
      unsigned long long real_idle_t2 = d2.idle + d2. iowait;

      if(total_t2 - total_t1 > 0){
         double usage = compute_percentage(total_t1, total_t2, real_idle_t1, real_idle_t2);
         printf("CPU : %.2f%%\r", usage);
         fflush(stdout);
      }
   };

   return 0;
}
