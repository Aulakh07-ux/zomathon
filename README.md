# Zomathon
This project implements a signal conditioning engine in C to correct noisy Kitchen Preparation Time (KPT) labels in food delivery systems. The system detects rider-induced marking bias, models live kitchen rush, handles simultaneous order clicking behavior, and reduces label noise using statistical techniques.
Problem Statement
Raw Kitchen Preparation Time (KPT) is computed as:
raw_kpt = for_time - confirm_time

However, the recorded Food Ready (FOR) timestamp may be distorted due to:
- Rider arrival bias
- Live kitchen rush (dine-in + other apps)
- Multiple orders marked ready at the same time

The goal is to reduce this noise and minimize error relative to true_kpt.
Synthetic Dataset Generator (Python)
A Python script generates realistic noisy data including:
- order_time (minutes of day)
- confirm_time (seconds)
- true_kpt (seconds)
- rider_arrival_time (seconds)
- restaurant_load (0–1)
- for_time (noisy timestamp)

The generator simulates rider bias, time-based rush, and bulk marking behavior.
Bias Detection Strategy
Delta is computed as:
delta = for_time - rider_arrival_time

We calculate mean and standard deviation of delta.
Bias condition:
|delta - mean_delta| < std_delta

If this condition holds, rider-induced bias is detected.
Live Rush Modeling
Rush is modeled dynamically using:
- order_time (minutes)
- restaurant_load

Time multipliers:
- Lunch (12–3 PM): 1.6x
- Dinner (6–10 PM): 1.8x
- Off-peak: 1.0x

Dynamic rush adjustment:
effective_load = restaurant_load × time_multiplier
rush_seconds = effective_load × 60

Use of Quicksort
Quicksort is used to efficiently sort delta values to analyze distribution patterns and detect clustering behavior in marking timestamps.

Average Time Complexity: O(n log n)
Worst Case: O(n^2)
Use of Quickselect
Quickselect is used to compute median or percentile thresholds of delta values without fully sorting the dataset.

Average Time Complexity: O(n)
Evaluation Metric (MSE)
Mean Squared Error (MSE) is used to evaluate correction performance.

MSE_before = average((raw_kpt - true_kpt)^2)
MSE_after  = average((corrected_kpt - true_kpt)^2)

Improvement % = ((MSE_before - MSE_after) / MSE_before) × 100

The objective is to ensure MSE_after < MSE_before.
Project Structure
synthetic_data_generator.py
synthetic_noisy_data_v2.csv
kpt_engine.c
README.docx
How to Run
1. Generate dataset:
   python synthetic_data_generator.py

2. Compile C engine:
   gcc kpt_engine.c -lm -o kpt_engine

3. Run:
   ./kpt_engine

The program outputs dataset statistics, MSE before correction, MSE after correction, and percentage improvement.
