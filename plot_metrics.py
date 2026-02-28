import matplotlib.pyplot as plt

def generate_pdf_graphs():
    # UPDATE THESE NUMBERS WITH WHAT THE C TERMINAL SPIT OUT
    mse_before = 2252117.015345
    mse_after = 31687.194961
    improvement = 98.59
    
    labels = ['Before (Current Zomato App)', 'After (Our C++ Denoising Engine)']
    values = [mse_before, mse_after]
    colors = ['#E23744', '#28A745'] # Zomato Red and Success Green

    plt.figure(figsize=(10, 6))
    bars = plt.bar(labels, values, color=colors, width=0.5)
    
    plt.title('ETA Prediction Error (Mean Squared Error)', fontsize=16, fontweight='bold')
    plt.ylabel('Error Variance (Lower is Better)', fontsize=12)
    
    for bar in bars:
        yval = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2, yval + (mse_before*0.02), f"{int(yval):,}", ha='center', fontsize=12, fontweight='bold')

    plt.text(0.5, mse_before * 0.5, f'{improvement}% Improvement\nin Accuracy!', ha='center', fontsize=16, color='black', bbox=dict(facecolor='white', alpha=0.8, edgecolor='none'))

    plt.tight_layout()
    plt.savefig('business_impact_graph.png', dpi=300)
    print("Graph saved as 'business_impact_graph.png'")

if __name__ == "__main__":
    generate_pdf_graphs()