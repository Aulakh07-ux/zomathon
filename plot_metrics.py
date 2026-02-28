import matplotlib.pyplot as plt

def generate_championship_graph():
    # Your final terminal outputs
    mse_before = 2252117.01 
    mse_after = 10406.47
    improvement = 99.54
    latency_ms = 12  # 0.012s total time
    
    labels = ['Before\n(Current App Signals)', 'After\n(Pure C Denoising Engine)']
    values = [mse_before, mse_after]
    colors = ['#E23744', '#28A745'] # Zomato Red and Success Green

    plt.figure(figsize=(10, 6.5))
    bars = plt.bar(labels, values, color=colors, width=0.4)
    
    plt.title('ETA Prediction Error (Mean Squared Error)', fontsize=16, fontweight='bold', pad=20)
    plt.ylabel('Error Variance (Lower is Better)', fontsize=12, fontweight='bold')
    
    # Put the exact numbers on top of the bars
    for bar in bars:
        yval = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2, yval + (mse_before*0.02), f"{int(yval):,}", ha='center', fontsize=12, fontweight='bold')

    # The 99.54% Improvement Badge
    plt.text(0.5, mse_before * 0.5, f'🔥 {improvement}% Improvement\nin Prediction Accuracy!', 
             ha='center', fontsize=15, fontweight='bold', color='black', 
             bbox=dict(facecolor='white', alpha=0.9, edgecolor='#28A745', boxstyle='round,pad=0.5', linewidth=2))

    # The 12ms Latency Flex at the bottom
    plt.text(0.5, mse_before * 0.85, f'⚡ Engine Algorithmic Latency: {latency_ms} ms\n(Easily beats Zomato\'s 300ms budget)', 
             ha='center', fontsize=11, style='italic', color='#555555',
             bbox=dict(facecolor='#f8f9fa', alpha=0.9, edgecolor='none', boxstyle='round,pad=0.5'))

    plt.tight_layout()
    plt.savefig('FINAL_business_impact_graph.png', dpi=300)
    print("Graph saved as 'FINAL_business_impact_graph.png'. Ready for the PDF!")

if __name__ == "__main__":
    generate_championship_graph()
