# ADE9000

This ade9000 module, implements the logic for interfacing with the ADE9000 chip, an integrated energy metering device from Analog Devices. The driver includes SPI communication, register configuration, waveform acquisition, FFT processing for power analysis, and interrupt/event management.

---

## Initialization and Configuration

- **System Parameter Setup**:
  - Reads transformer ratios and calculates internal scale factors.
  - Prepares gain and configuration registers based on user parameters.

- **Register Access**:
  - `ade9000_read_register`, `ade9000_write_register`, and `ade9000_check_written_register` handle low-level SPI read/write with integrity checks.
  - Configuration includes setting high-pass filters, current/voltage gains, ADC redirect, and waveform capture settings.

- **Waveform Buffer**:
  - Configured to capture voltage/current channels at 8kSPS.
  - Uses burst SPI reads to access sampled waveform data stored in the ADE9000's buffer.

 ### Real-Time Data Capture

- **IRQ-Driven Buffer Handling**:
  - Interrupts trigger when a buffer page is full (either page 7 or 15).
  - Data is read and reassembled into full 1024-sample blocks.

- **Waveform Sample Processing**:
  - Extracts per-sample voltage and current, converts them to physical values using configured scale factors.
  - Fills RMS buffers used for apparent power calculation.

## Power Calculation

The driver calculates:

- S (apparent power) via RMS over multiple cycles
- P (active power) and Q (reactive power) via FFT and interpolation.

All calculations operate on 1024-sample buffers (defined by SAMPLES_POWER_CALC), chosen as a power-of-two for optimal FFT performance:

 - FFT libraries like CMSIS-DSP, KISS FFT, and FFTW are optimized for lengths that are powers of two.
 - Sizes like 512, 1024, and 2048 are computationally efficient and widely supported.
 - Using 1024 allows high resolution (7.8125 Hz/bin at 8kSPS) without being too heavy.

### Apparent Power (S)

For S (apparent power), we cannot use the complete buffer (1024), bacause we have to make sure we add complete cycles into the buffer:

So depending on the frequency the number of stores samples is calculated:

| Grid Frequency | Samples per Cycle | Chosen Buffer full cycles | Total Samples |
|----------------|-------------------|------------------------------|----------------|
| 50 Hz          | 160               | 6                            | 960            |
| 60 Hz          | 400        | 2                            | 800            |

RMS(V) × RMS(I) is computed continuously using a calculated sample window (960 or 800).

### Active and Reactive Power (P, Q)

To compute P and Q with spectral resolution, the system uses an FFT-based approach. Three core techniques are applied: zero padding, Hamming windowing, and interpolation.

#### Zero Padding

If the number of valid waveform samples (e.g., 960) is less than 1024:
- The remaining buffer space is filled with zeros, symmetrically (before and after the signal).
- This ensures compatibility with the 1024-point FFT.  

Problems: 

- Zero-padding introduces discontinuities at the edges, violating the FFT’s assumption of signal periodicity.
- This leads to spectral leakage (energy spreading to adjacent bins) and distorted amplitude of harmonics.

#### Hamming Window

To mitigate the distortion caused by zero-padding, a Hamming window is applied across the entire 1024 buffer:

hamming_window[n] = 0.54 - 0.46 * cos(2πn / (N - 1)),  n = 0...1023

Each sample is multiplied by this window before performing the FFT.

- Reduces edge discontinuities: Smooths the signal edges so they taper toward zero, reducing spectral  leakage.
- Minimizes FFT noise: Makes the transition from real samples to zeros gradual, not abrupt.
- Improves power estimation: Leads to more stable and reliable P/Q calculations using bin dot-products.

After Hamming window is applied to the buffer, FFT calculations are done. From the resultant complex values P and Q power are calculated.

- P (Active Power): P = Re(V)×Re(I) + Im(V)×Im(I)
- Q (Reactive Power): Q = Im(V)×Re(I) - Re(V)×Im(I)

#### Interpolation

For performance reasons, FFT is alculated every 128 samples, not at every sample. However, the FFT result only strictly represents the last sample of the block. To assign a single P and Q value to all 128 samples would create artificial steps and delays in the waveform evolution. To reduce this effect, interpolation is used. The previous and current FFT result is sotred and then for each 128 samples interpolation is applied:

P[i] = (1 - α) * P_prev + α * P_curr  
Q[i] = (1 - α) * Q_prev + α * Q_curr  
α = i / (N - 1), with N = 128

This results in a smooth transition between blocks and a more accurate per-sample representation of the evolution of active and reactive power






