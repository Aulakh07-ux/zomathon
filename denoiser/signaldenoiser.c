#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ORDERS 15000

// ==========================================
// 1. DATA STRUCTURE
// ==========================================
typedef struct {
    long order_id;
    int merchant_id;
    double confirm_time;
    double true_kpt;
    double rider_arrival_time;
    double restaurent_load;
    double for_time;
    double order_time;
    double zomato_baseline;
    double sensor_kpt;
} ordersignal;

// ==========================================
// 2. QUICKSELECT (Median in O(n))
// ==========================================
void swap(double *a, double *b) {
    double temp = *a;
    *a = *b;
    *b = temp;
}

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

double quickselect(double arr[], int left, int right, int k) {
    if (k > 0 && k <= right - left + 1) {
        int index = partition(arr, left, right);

        if (index - left == k - 1)
            return arr[index];

        if (index - left > k - 1)
            return quickselect(arr, left, index - 1, k);

        return quickselect(arr, index + 1, right,
                           k - (index - left + 1));
    }
    return 0.0;
}

// ==========================================
// 3. DELTA STATS
// ==========================================
void delta_stats(ordersignal orders[], int count,
                 double *median, double *std_dev) {

    if (count == 0) {
        *median = 0;
        *std_dev = 0;
        return;
    }

    double *deltas = (double *)malloc(count * sizeof(double));

    for (int i = 0; i < count; i++) {
        deltas[i] = orders[i].for_time -
                    orders[i].rider_arrival_time;
    }

    *median = quickselect(deltas, 0, count - 1,
                          (count / 2) + 1);

    double var_sum = 0.0;
    for (int i = 0; i < count; i++) {
        var_sum += pow(deltas[i] - *median, 2);
    }

    *std_dev = sqrt(var_sum / count);

    free(deltas);
}

// ==========================================
// 4. RUSH MODEL
// ==========================================
double compute_time_multiplier(double order_time) {
    int minutes = (int)order_time;

    if (minutes >= 720 && minutes <= 900)       // 12–3 PM
        return 1.6;

    if (minutes >= 1080 && minutes <= 1320)     // 6–10 PM
        return 1.8;

    return 1.0;
}

double compute_dynamic_rush_seconds(ordersignal o) {
    double multiplier =
        compute_time_multiplier(o.order_time);

    double effective_load =
        o.restaurent_load * multiplier;

    return effective_load * 40.0;
}

// ==========================================
// 5. PREDICTION ENGINE
// ==========================================
double compute_raw_kpt(ordersignal o) {
    // IoT sensor preferred, fallback to baseline
    if (o.sensor_kpt > 0)
        return o.sensor_kpt;

    return o.zomato_baseline;
}

int bias_detection(ordersignal o,
                   double median_delta,
                   double std_delta) {

    double delta =
        o.for_time - o.rider_arrival_time;

    if (delta > (median_delta +
                (std_delta * 0.5)))
        return 1;

    return 0;
}

double correct_kpt(ordersignal o,
                   double median_delta,
                   double std_delta) {

    double raw_kpt =
        compute_raw_kpt(o);

    double delta =
        o.for_time - o.rider_arrival_time;

    int is_biased =
        bias_detection(o,
                       median_delta,
                       std_delta);

    // Case 1: Batch marking bias
    if (is_biased) {
        raw_kpt -= delta;
    }

    // Case 2: Kitchen congestion
    if (!is_biased &&
        o.restaurent_load > 6) {

        raw_kpt -=
            compute_dynamic_rush_seconds(o);
    }

    // Safety fallback
    if (raw_kpt <= 0)
        return o.zomato_baseline;

    return raw_kpt;
}

double compute_final_prediction(ordersignal o,
                                double median_delta,
                                double std_delta) {

    return correct_kpt(o,
                       median_delta,
                       std_delta);
}

// ==========================================
// 6. MAIN
// ==========================================
int main() {

    FILE *file =
        fopen("realistic_chaos_data.csv", "r");

    if (!file) {
        printf("ERROR: CSV not found.\n");
        return 1;
    }

    ordersignal *orders =
        (ordersignal *)malloc(
            MAX_ORDERS *
            sizeof(ordersignal));

    char line[512];
    int count = 0;

    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file)
           && count < MAX_ORDERS) {

        sscanf(line,
        "%ld,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
        &orders[count].order_id,
        &orders[count].merchant_id,
        &orders[count].confirm_time,
        &orders[count].true_kpt,
        &orders[count].rider_arrival_time,
        &orders[count].restaurent_load,
        &orders[count].for_time,
        &orders[count].order_time,
        &orders[count].zomato_baseline,
        &orders[count].sensor_kpt);

        count++;
    }

    fclose(file);

    clock_t start_time = clock();

    double median_delta, std_delta;

    delta_stats(orders, count,
                &median_delta,
                &std_delta);

    double mae_baseline = 0.0;
    double mae_optimized = 0.0;

    for (int i = 0; i < count; i++) {

        double optimized_pred =
            compute_final_prediction(
                orders[i],
                median_delta,
                std_delta);

        mae_baseline += fabs(
            orders[i].zomato_baseline -
            orders[i].true_kpt);

        mae_optimized += fabs(
            optimized_pred -
            orders[i].true_kpt);
    }

    clock_t end_time = clock();

    double latency =
        ((double)(end_time - start_time)
         / CLOCKS_PER_SEC) * 1000.0;

    mae_baseline =
        (mae_baseline / count) / 60.0;

    mae_optimized =
        (mae_optimized / count) / 60.0;

    printf("\n=== ZOMATHON SYSTEM TEST ===\n");
    printf("Processed %d orders\n", count);
    printf("Latency: %.3f ms\n", latency);
    printf("Baseline MAE : %.2f minutes\n",
           mae_baseline);
    printf("Optimized MAE: %.2f minutes\n",
           mae_optimized);

    if (mae_baseline > 0) {
        printf("Improvement  : %.2f%%\n",
        ((mae_baseline -
          mae_optimized)
         / mae_baseline) * 100);
    }

    printf("===========================\n");

    free(orders);
    return 0;
}