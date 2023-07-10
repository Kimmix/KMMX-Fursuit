## BLE Controlable
Feature
    - Toggle BOOP: True/False
    - Toggle GooglyEye: True/False
    - Face Expression

Display
    - Brightness: 0 - 100
    - Toggle draw color: acc/hypno

Viseme
    - NOISE_THRESHOLD: 300
    - SMOOTHING_ALPHA: 0 - 1.0
    Loudness
        - level1: 1.8
        - level2: 1.6
        - level3: 1.4
    Freq ranges
        - AH_MIN: 600
        - AH_MAX: 1200
        - EE_MIN: 1000
        - EE_MAX: 2000
        - OH_MIN: 1800
        - OH_MAX: 2800
        - OO_MIN: 2600
        - OO_MAX: 3600
        - TH_MIN: 3400
        - TH_MAX: 4000
    Amplitude modifier
        - ah_amplitude: 0.5
        - ee_amplitude: 0.6
        - oh_amplitude: 1.8
        - oo_amplitude: 2.0
        - th_amplitude: 2.3
    Decay
        - decay_rate: 0.95
        - hold_time: 5000

## Viseme combination
    - AH
    - EE
    - OH
    - OO
    - TH
    - AHEE
    - AHOH
    - AHOO
    - AHTH
    - EEOH
    - EEOO
    - EETH
    - OHOO
    - OHTH
    - OOTH

# TO-DO
- Viseme
    - Add dynamic noise threhold funtion
    - Cross viseme

- LEXMatrixDisplay
    - Update MVC
    - frame interpolate