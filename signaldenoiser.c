#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ORDERS 10000

typedef struct {
    long order_id;
    int merchant_id;
    double confirm_time;
    double true_kpt;
    double rider_arrival_time;
    double restaurent_load;
    double for_time;
    double order_time;
} ordersignal;

/* -------------------- QUICKSELECT FOR MEDIAN -------------------- */
// Function to swap two doubles
void swap(double *a, double *b) {
    double temp = *a;
    *a = *b;
    *b = temp;
}

// Partition function for QuickSelect
int partition(double arr[], int left, int right) {
    double pivot = arr[right];
    int i = left;
    for (int j = left; j <= right - 1; j++) {
        if (arr[j] <= pivot) {
            swap(&arr[i], &arr[j]);
            i++;
        }
    }
    swap(&arr[i], &arr[right]);
    return i;
}

// O(n) QuickSelect to find the kth smallest element
double quickselect(double arr[], int left, int right, int k) {
    if (k > 0 && k <= right - left + 1) {
        int index = partition(arr, left, right);
        if (index - left == k - 1) return arr[index];
        if (index - left > k - 1) return quickselect(arr, left, index - 1, k);
        return quickselect(arr, index + 1, right, k - (index - left + 1));
    }
    return 0;
}

/* -------------------- RAW KPT -------------------- */
double compute_raw_kpt(ordersignal o) {
    return o.for_time - o.confirm_time;
}

/* -------------------- BIAS DETECTION -------------------- */
int bias_detection(ordersignal a,double median_delta,double std_delta) {
    double delta = a.for_time - a.rider_arrival_time;
    // Bias is detected if the gap is significantly smaller than the historical median
    if (fabs(delta - median_delta) < std_delta)
        return 1;   // biased
    else
        return 0;   // not biased
}

/* -------------------- DELTA STATS -------------------- */
void delta_stats(ordersignal orders[],int count,double *median,double *std_dev) {
    if (count == 0) {
        *median = 0;
        *std_dev = 0;
        return;
    }

    // Allocate memory for QuickSelect
    double *deltas = (double *)malloc(count * sizeof(double));
    for (int i = 0; i < count; i++) {
        deltas[i] = orders[i].for_time - orders[i].rider_arrival_time;
    }

    // Find Median using QuickSelect O(n)
    *median = quickselect(deltas, 0, count - 1, (count / 2) + 1);

    double variance_sum = 0.0;
    for (int i = 0; i < count; i++) {
        variance_sum += pow(deltas[i] - *median, 2);
    }

    *std_dev = sqrt(variance_sum / count);
    free(deltas);
}

/*---------- dynamic rush --------------*/
double compute_time_multiplier(double order_time)
{
    int minutes = (int)order_time;
    if (minutes >= 720 && minutes <= 900) return 1.6; 
    if (minutes >= 1080 && minutes <= 1320) return 1.8; 
    return 1.0;
}

double compute_dynamic_rush_seconds(ordersignal o)
{
    double time_multiplier = compute_time_multiplier(o.order_time);
    double effective_load = o.restaurent_load * time_multiplier;
    return effective_load * 60.0; 
}

/*------------------ correct kpt -------------------*/
double correct_kpt(ordersignal o,double median_delta,double std_delta){
    double raw_kpt = compute_raw_kpt(o);   
    double delta = o.for_time - o.rider_arrival_time;
    int biased = bias_detection(o,median_delta,std_delta);

    if(biased) {
        raw_kpt -= delta; 
    }
    raw_kpt += compute_dynamic_rush_seconds(o);

    if (raw_kpt < 0) raw_kpt = 0;
    return raw_kpt;
}

/* -------------------- CSV LOADER -------------------- */
int load_dataset(const char *filename,ordersignal orders[],int max_size) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file.\n");
        return 0;
    }
    char line[256];
    int count = 0;
    fgets(line, sizeof(line), file); // Skip header

    while (fgets(line, sizeof(line), file) != NULL && count < max_size) {
        sscanf(line, "%ld,%d,%lf,%lf,%lf,%lf,%lf,%lf",
               &orders[count].order_id, &orders[count].merchant_id,
               &orders[count].confirm_time, &orders[count].true_kpt,
               &orders[count].rider_arrival_time, &orders[count].restaurent_load,
               &orders[count].for_time, &orders[count].order_time);
        count++;
    }
    fclose(file);
    return count;
}

/* -------------------- MAIN -------------------- */
int main() {
    const char *filename = "synthetic_noisy_data_v2.csv";
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) return 1;

    int total_lines = 0;
    char ch;
    while((ch = fgetc(fp)) != EOF) if(ch == '\n') total_lines++;
    fclose(fp);

    int actual_capacity = (total_lines > 1) ? total_lines - 1 : MAX_ORDERS;
    ordersignal *orders = (ordersignal *)malloc(actual_capacity * sizeof(ordersignal));
    if (orders == NULL) return 1;

    int count = load_dataset(filename, orders, actual_capacity);
    if (count == 0) { free(orders); return 1; }

    printf("Loaded %d orders\n", count);
    double median_delta, std_delta;
    delta_stats(orders,count,&median_delta,&std_delta);

    printf("Median Delta: %f\n", median_delta);
    printf("Std Dev Delta: %f\n", std_delta);

    double mse_before = 0.0, mse_after = 0.0;
    for (int i = 0; i < count; i++) {
        double raw = compute_raw_kpt(orders[i]);
        double corrected = correct_kpt(orders[i],median_delta,std_delta);
        mse_before += pow(raw - orders[i].true_kpt, 2);
        mse_after += pow(corrected - orders[i].true_kpt, 2);
    }

    printf("MSE Before: %f\n", mse_before / count);
    printf("MSE After : %f\n", mse_after / count);

    if (mse_before > 0) {
        printf("Improvement: %.2f%%\n", ((mse_before - mse_after) / mse_before) * 100.0);
    }

    free(orders); 
    return 0;
}