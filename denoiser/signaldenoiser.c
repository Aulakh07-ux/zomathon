#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

#define MAX_ORDERS 15000

// ==========================================
// 1. DATA STRUCTURE (Matches Python CSV 1:1)
// ==========================================
typedef struct {
    long order_id;
    int merchant_id;
    double confirm_time;
    double true_kpt;
    double rider_arrival_time;
    double restaurent_load;   // FEATURE: Kitchen Load
    double for_time;
    double order_time;        // FEATURE: Time of Day
    double zomato_baseline;
    double sensor_kpt;        // FEATURE: IoT Hardware
} ordersignal;

// ==========================================
// 2. O(n) QUICKSELECT (Sub-millisecond Speed)
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
        if (index - left == k - 1) return arr[index];
        if (index - left > k - 1) return quickselect(arr, left, index - 1, k);
        return quickselect(arr, index + 1, right, k - (index - left + 1));
    }
    return 0;
}

// ==========================================
// 3. STATS & DYNAMIC KITCHEN RUSH PENALTY
// ==========================================
void delta_stats(ordersignal orders[], int count, double *median, double *std_dev) {
    if (count == 0) return;
    
    double *deltas = (double *)malloc(count * sizeof(double));
    for (int i = 0; i < count; i++) {
        deltas[i] = orders[i].for_time - orders[i].rider_arrival_time;
    }
    
    *median = quickselect(deltas, 0, count - 1, (count / 2) + 1);
    
    double var_sum = 0.0;
    for (int i = 0; i < count; i++) {
        var_sum += pow(deltas[i] - *median, 2);
    }
    
    *std_dev = sqrt(var_sum / count);
    free(deltas);
}

double compute_time_multiplier(double order_time) {
    int minutes = (int)order_time;
    // Lunch Peak (12 PM - 3 PM)
    if (minutes >= 720 && minutes <= 900) {
        return 1.6; 
    }
    // Dinner Peak (6 PM - 10 PM)
    if (minutes >= 1080 && minutes <= 1320) {
        return 1.8; 
    }
    return 1.0;
}

double compute_dynamic_rush_seconds(ordersignal o) {
    double time_multiplier = compute_time_multiplier(o.order_time);
    double effective_load = o.restaurent_load * time_multiplier;
    return effective_load * 40.0; // Subtracts 40 seconds per overlapping order
}

// ==========================================
// 4. THE 3-TIERED HYBRID DENOISING LOGIC
// ==========================================
double compute_final_prediction(ordersignal o, double median_delta, double std_delta) {
    // TIER 1: HARDWARE IOT SENSOR
    // If the sensor didn't drop a packet (-1.0), it is the absolute ground truth.
    if (o.sensor_kpt != -1.0) {
        return o.sensor_kpt; 
    }
    
    // TIER 2: SOFTWARE DENOISING (No hardware available or network failed)
    double raw_kpt = o.for_time - o.confirm_time;
    double delta = o.for_time - o.rider_arrival_time;
    int is_biased = 0;
    
    // FILTER A: Catch "Batch Marking" (Massive gap between prep and rider)
    if (delta > (median_delta + (std_delta * 0.5))) {
        raw_kpt -= delta; 
        is_biased = 1;
    }
    
    // FILTER B: Catch "Kitchen Congestion" (Managers delaying iPad clicks)
    if (is_biased == 0 && o.restaurent_load > 6) {
        raw_kpt -= compute_dynamic_rush_seconds(o);
    }
    
    // TIER 3: HISTORICAL FALLBACK
    // If the math results in a negative ETA, fallback to Zomato's baseline average
    if (raw_kpt <= 0) {
        return o.zomato_baseline;
    } else {
        return raw_kpt;
    }
}

// ==========================================
// 5. MAIN EXECUTION (CSV Reader & Latency Timer)
// ==========================================
int main() {
    FILE *file = fopen("realistic_chaos_data.csv", "r");
    if (!file) {
        printf("ERROR: Could not find CSV. Run the Python script first!\n");
        return 1;
    }

    ordersignal *orders = (ordersignal *)malloc(MAX_ORDERS * sizeof(ordersignal));
    char line[512]; 
    int count = 0;
    
    if (fgets(line, sizeof(line), file) == NULL) {
    fprintf(stderr, "Error reading header line\n");
    fclose(file);
    free(orders);
    return 1;
    }
    // Scan exactly 10 columns
    while (fgets(line, sizeof(line), file) != NULL && count < MAX_ORDERS) {
        sscanf(line, "%ld,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
               &orders[count].order_id, &orders[count].merchant_id,
               &orders[count].confirm_time, &orders[count].true_kpt,
               &orders[count].rider_arrival_time, &orders[count].restaurent_load,
               &orders[count].for_time, &orders[count].order_time,
               &orders[count].zomato_baseline, &orders[count].sensor_kpt);
        count++;
    }
    fclose(file);

    // --- START LATENCY BENCHMARK ---
    clock_t start_time = clock(); 
    
    double median_delta, std_delta;
    delta_stats(orders, count, &median_delta, &std_delta);

    double mae_baseline = 0.0, mae_optimized = 0.0;

    for (int i = 0; i < count; i++) {
        double optimized_pred = compute_final_prediction(orders[i], median_delta, std_delta);
        
        // Calculate Mean Absolute Error (MAE) for both models
        mae_baseline += fabs(orders[i].zomato_baseline - orders[i].true_kpt);
        mae_optimized += fabs(optimized_pred - orders[i].true_kpt);
    }

    // --- STOP LATENCY BENCHMARK ---
    clock_t end_time = clock(); 
    double latency = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

    mae_baseline = (mae_baseline / count) / 60.0; // Convert to minutes
    mae_optimized = (mae_optimized / count) / 60.0; // Convert to minutes

    printf("\n=== ZOMATHON SYSTEM TEST RESULTS ===\n");
    printf("Processed %d chaotic orders.\n", count);
    printf("Algorithmic Latency: %.3f ms\n", latency);
    printf("------------------------------------\n");
    printf("Baseline Zomato Error (MAE) : %.2f minutes off per order\n", mae_baseline);
    printf("Hybrid System Error (MAE)   : %.2f minutes off per order\n", mae_optimized);
    printf("Improvement                 : %.2f%%\n", ((mae_baseline - mae_optimized)/mae_baseline) * 100);
    printf("====================================\n\n");

    free(orders); 
    return 0;
}