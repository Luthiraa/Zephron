#include <stdio.h>
#include <stdlib.h>

typedef struct {
    float Kp;
    float Ki;
    float Kd;
} PID;

int main(void) {
    PID p;
    printf("Enter Kp Ki Kd:\n");
    if (scanf("%f %f %f", &p.Kp, &p.Ki, &p.Kd) != 3)
        return 1;

    FILE *f = fopen("pid_params.txt", "w");
    if (!f)
        return 1;
    fprintf(f, "%f %f %f", p.Kp, p.Ki, p.Kd);
    fclose(f);

    return 0;
}
