# Zomathon
📌 Project Title

Zomato KPT Solution DevX
Signal conditioning engine for correcting noisy Kitchen Preparation Time (KPT) labels in food delivery systems.

🧠 Project Overview

Kitchen Preparation Time (KPT) is a crucial metric in food delivery systems that measures the time between order confirmation and food readiness.
In real delivery environments, recorded KPT labels can be noisy due to factors such as rider arrival bias, simultaneous ordering, and peak kitchen load. This project implements a signal conditioning solution to reduce noise in KPT measurements, producing more reliable preparation time estimates.

🚀 Key Features
Raw KPT noise reduction engine (C implementation)
Python utilities for generating synthetic datasets
Visualization of performance and noise correction effects
Designed for integration with real delivery systems

📁 Repository Contents
📦 Zomato_KPT_Solution_DevX
 ┣ 📂 .vscode
 ┣ 📂 denoiser
 ┣ 📜 .gitignore
 ┣ 📜 LICENSE (GPL-3.0)
 ┣ 📜 README.md
 ┣ 📜 plot_metrics.py
🛠️ Tools & Languages Used
Technology	Purpose
C	Core signal conditioning engine implementation
Python	Scripted metrics generation and visualization
GPL-3.0 License	Open source licensing
📐 Problem Statement

Recording Kitchen Preparation Time (KPT) without distortion is hard. Noise in raw labels arises because:

Rider arrival timing can bias “ready” timestamps

A kitchen gets busier during peak hours, adding distortions

Multiple orders confirmed together distort genuine preparation intervals

This system applies statistical techniques to detect bias and correct raw KPT values closer to a true estimate.

📈 How It Works

Synthetic Data Generation
Generate a dataset that mimics real kitchen and rider behavior, including noisy KPT labels.

Signal Conditioning Engine
C engine analyzes noisy time measurements and produces corrected KPT estimates using statistical bias detection.

Evaluation & Visualization
Python plotting scripts evaluate correction quality (before vs after).

📌 Usage Instructions
1. Building the C Engine
gcc kpt_engine.c -lm -o kpt_engine

Links math library (-lm) and produces executable kpt_engine

2. Running the Engine
./kpt_engine

This outputs corrected KPT metrics alongside baseline raw values.

3. Plotting Metrics (Python)

In the repository root, run:

python plot_metrics.py

This script reads stored results and creates visual comparisons to assess noise reduction performance.

📊 Expected Outputs

Corrected KPT values with reduced variance

MSE improvement over raw measurements

Plots showing before/after conditioning performance

🧪 Examples
Measurement	Raw KPT	Corrected KPT
Order A	15 min	12.4 min
Order B	22 min	18.7 min

Your actual values may vary depending on synthetic data generator parameters.

📜 License

This repository is licensed under the GPL-3.0 License. Refer to the LICENSE file for details.
