import pandas as pd
import numpy as np
import time

def generate_perfect_zomato_data():
    print("Initializing Zomato Data Simulator...")
    
    # Matching the C engine's capacity limits
    NUM_ORDERS = 5000  
    NUM_MERCHANTS = 200 
    
    # Base midnight epoch time to start our simulated 24-hour day
    base_epoch_time = int(time.time())
    
    data = []
    
    print(f"Generating {NUM_ORDERS} dynamic orders with Rush Windows...")
    for i in range(NUM_ORDERS):
        merchant_id = np.random.randint(1, NUM_MERCHANTS + 1)
        
        # 1. Generate order_time (Minute of the day: 0 to 1439)
        order_time = np.random.randint(0, 1440)
        
        # 2. Match your teammate's EXACT C-code rush logic for realistic data
        if 720 <= order_time <= 900:      # Lunch: 12 PM - 3 PM
            rush_multiplier = 1.6
        elif 1080 <= order_time <= 1320:  # Dinner: 6 PM - 10 PM
            rush_multiplier = 1.8
        else:                             # Normal hours
            rush_multiplier = 1.0
            
        # 3. Generate Base Restaurant Load
        base_load = np.random.uniform(0.1, 0.7)
        restaurent_load = min(base_load * rush_multiplier, 1.5) 
        
        # 4. Compute true_kpt (Ground Truth in seconds)
        base_kpt_seconds = np.random.randint(900, 1500) 
        true_kpt = base_kpt_seconds + int(restaurent_load * 600)
        
        # 5. Confirm Time (Epoch seconds)
        confirm_time = base_epoch_time + (order_time * 60) + np.random.randint(0, 59)
        
        # 6. Rider Arrival (Arrives around when food is done, +/- a few minutes)
        rider_arrival_time = confirm_time + true_kpt + np.random.randint(-120, 300)
        
        # Base "Food Ready" (for_time) manual click (Honest scenario)
        for_time = confirm_time + true_kpt + np.random.randint(10, 30)
        
        data.append({
            'order_id': i + 1,
            'merchant_id': merchant_id,
            'order_time': float(order_time),       
            'confirm_time': float(confirm_time),
            'true_kpt': float(true_kpt),
            'rider_arrival_time': float(rider_arrival_time),
            'restaurent_load': round(float(restaurent_load), 3),
            'for_time': float(for_time)
        })

    df = pd.DataFrame(data)

    # --- INJECTING THE CORRELATED NOISE & BIAS ---
    print("Injecting Rider Bias and Congestion Noise...")
    
    # 1. Rider Bias (40% chance): Merchant clicks ready ONLY when rider arrives
    # Your teammate's C code specifically looks for this tight delta!
    rider_bias_mask = np.random.rand(len(df)) < 0.40
    df.loc[rider_bias_mask, 'for_time'] = df.loc[rider_bias_mask, 'rider_arrival_time'] + np.random.uniform(0.5, 2.5)

    # 2. Congestion Noise: If the kitchen is slammed, they are slower to click the iPad
    df['for_time'] = df['for_time'] + (df['restaurent_load'] * 120)

    # 3. Batch Marking (The lazy manager bias)
    print("Injecting Batch-Marking Bias...")
    batch_merchants = np.random.choice(df['merchant_id'].unique(), 40, replace=False)
    for m in batch_merchants:
        m_orders = df[df['merchant_id'] == m].sort_values('order_time').head(4)
        if len(m_orders) >= 2:
            # Force the lazy manager to delay the "Ready" click by an extra 15 minutes!
            batch_ts = m_orders['for_time'].max() + 900
            df.loc[m_orders.index, 'for_time'] = batch_ts

    # --- FINAL CLEANUP & EXPORT ---
    # Sort chronologically
    df = df.sort_values('confirm_time').reset_index(drop=True)
    
    # 🚨 THE CRITICAL FIX: Reorder columns to perfectly match the C engine's sscanf 🚨
    # If this isn't exact, the C code will crash or output garbage numbers.
    df = df[['order_id', 'merchant_id', 'confirm_time', 'true_kpt', 'rider_arrival_time', 'restaurent_load', 'for_time', 'order_time']]
    
    # Cast IDs to integers so they format cleanly in the CSV
    df['order_id'] = df['order_id'].astype(int)
    df['merchant_id'] = df['merchant_id'].astype(int)
    
    # Format floats to 2 decimal places to keep the CSV clean for C parsing
    filename = 'synthetic_noisy_data_v2.csv'
    df.to_csv(filename, index=False, float_format='%.2f')
    
    print(f"\nSUCCESS! '{filename}' created.")
    print("-> Column order perfectly matches the C script.")
    print("-> Give this to your C teammate to calculate the MSE Improvement!")

if __name__ == "__main__":
    generate_perfect_zomato_data()
