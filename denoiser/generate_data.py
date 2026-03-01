import numpy as np
import pandas as pd

def generate_championship_dataset():
    print("Initializing Full-Scale Zomathon Chaos Generator...")
    np.random.seed(42)

    N = 10000 
    num_merchants = 50

    data = pd.DataFrame()

    # ==========================================
    # STEP 1: ORDER ROUTING & TIME PROFILES
    # ==========================================
    data["order_id"] = np.arange(1, N + 1)
    data["merchant_id"] = np.random.randint(1, num_merchants + 1, N)
    
    # Simulate a full 24-hour cycle (in minutes)
    data["order_time"] = np.random.uniform(0, 1440, N)
    data["confirm_time"] = data["order_time"] * 60 + np.random.uniform(60, 180, N) # +1 to 3 mins
    data["hour"] = (data["order_time"] // 60).astype(int)
    
    # Flag peak lunch (12-2 PM) and peak dinner (7-9 PM)
    data["is_peak"] = data["hour"].isin([12, 13, 14, 19, 20, 21]).astype(int)

    # ==========================================
    # STEP 2: REAL-WORLD KITCHEN CONGESTION
    # ==========================================
    # Visible Zomato Load vs. Hidden Walk-in Load (Poisson Distribution)
    data["restaurent_load"] = np.random.poisson(lam=3 + (4 * data["is_peak"]))
    hidden_load = np.random.poisson(lam=2 + (3 * data["is_peak"]))

    # ==========================================
    # STEP 3: TRUE PREP TIME & KITCHEN MELTDOWNS
    # ==========================================
    # Base prep is 15 mins. Add 40s per Zomato order, 50s per hidden order.
    base_kpt = np.random.normal(15, 3, N) * 60 
    data["true_kpt"] = base_kpt + (data["restaurent_load"] * 40) + (hidden_load * 50) + np.random.normal(0, 120, N)
    
    # 5% of the time, the kitchen completely crashes (dropping food, running out of stock)
    meltdown_mask = np.random.rand(N) < 0.05
    data.loc[meltdown_mask, "true_kpt"] += np.random.uniform(480, 900, meltdown_mask.sum()) # Adds 8 to 15 mins!
    data["true_kpt"] = data["true_kpt"].clip(lower=300) # Minimum 5 mins prep
    
    actual_ready_time = data["confirm_time"] + data["true_kpt"]

    # ==========================================
    # STEP 4: THE HUMAN BIAS (Corrupted App Signal)
    # ==========================================
    for_time = []
    for i, row in data.iterrows():
        # If the kitchen is super busy, managers batch-mark or delay clicks drastically
        if row["restaurent_load"] > 6:
            noise = np.random.uniform(120, 300) 
        else:
            noise = np.random.uniform(-60, 120)
        for_time.append(actual_ready_time[i] + noise)
    
    data["for_time"] = for_time

    # ==========================================
    # STEP 5: RIDER ARRIVAL DYNAMICS
    # ==========================================
    # Riders arrive around 8 mins (480s) after order confirmation, with traffic variance
    travel_time = np.random.normal(480, 120, N)
    data["rider_arrival_time"] = data["confirm_time"] + travel_time

    # ==========================================
    # STEP 6: ZOMATO'S CURRENT BASELINE
    # ==========================================
    # What Zomato's current ML model guesses (Historical average of that specific merchant)
    data["zomato_baseline"] = data.groupby("merchant_id")["true_kpt"].transform("mean")

    # ==========================================
    # STEP 7: IOT HARDWARE SIGNAL (With Dropouts)
    # ==========================================
    # The Smart Counter captures near-perfect data, but hardware networks fail 5% of the time
    sensor_dropout = np.random.rand(N) < 0.05
    data["sensor_kpt"] = data["true_kpt"] + np.random.uniform(0, 30, N) 
    data.loc[sensor_dropout, "sensor_kpt"] = -1.0 # The -1 tells the C Engine to use the software fallback!

    # ==========================================
    # STEP 8: EXPORT FOR THE C ENGINE
    # ==========================================
    # Exporting exactly the 10 columns the C Engine's sscanf expects
    export_df = data[['order_id', 'merchant_id', 'confirm_time', 'true_kpt', 'rider_arrival_time', 'restaurent_load', 'for_time', 'order_time', 'zomato_baseline', 'sensor_kpt']]
    export_df.to_csv('realistic_chaos_data.csv', index=False, float_format='%.2f')
    
    print(f"✅ Generated {N} hyper-realistic orders.")
    print("✅ Exported 'realistic_chaos_data.csv'. Ready for C Engine processing!")

if __name__ == "__main__":
    generate_championship_dataset()
