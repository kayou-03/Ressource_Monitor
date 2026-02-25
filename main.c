#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
} CPUData;

void read_cpu_data(CPUData *c){
   FILE *f = fopen("/proc/stat", "r");
   if(!f){
      perror("Error while opening the /proc/stat file\n");
      exit(EXIT_FAILURE);
   };

   int scanned =fscanf(f, "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
          &c->user, &c->nice, &c->system, &c->idle, &c->iowait, &c->irq, &c->softirq, &c->steal, &c->guest, &c->guest_nice);

   if(scanned < 10){
      fprintf(stderr, "Failed to scan the file properly, number of item scanned : %d (10 expected) \n", scanned);
      fclose(f);
      exit(EXIT_FAILURE);
   };
   
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
