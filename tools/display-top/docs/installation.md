# **Enabling MMIO and DPCD Register Reads in DisplayTop**  

DisplayTop requires JSON files for **MMIO** and **DPCD register data** to accurately interpret register values. These JSON files must be generated before running the tool.  

---

## **MMIO Register JSON Generation**  

The **Memory-Mapped I/O (MMIO) register JSON** provides details on register addresses and bitfields. Since MMIO register information is typically stored in **BXML files**, you need to convert them into JSON format.  

### **Steps to Generate MMIO JSON**  
1. **Obtain BXML files**  
   - These files contain register descriptions and bitfields.  
   - Ensure you have the latest version of the BXML register definitions.  

2. **Run the conversion script**  
   - A Python script (`bxml_to_json.py`) is used to process the BXML files.  
   - Navigate to the script’s directory:  
     ```sh
     cd scripts/
     ```
   - Run the script to generate `mmio_registers.json`:  
     ```sh
     python bxml_to_json.py
     ```
   - This script extracts register addresses, names, and bitfield details into a structured JSON format.  

3. **Verify the output**  
   - The generated JSON file should be placed in the expected location (e.g., `data/mmio_registers.json`).  
   - Open the file to ensure correctness:  
     ```sh
     cat data/mmio_registers.json | jq .
     ```

---

## **DPCD Register JSON Generation**  

The **DisplayPort Configuration Data (DPCD) register JSON** is derived from `drm_dp.h`, which contains important **DisplayPort register definitions**.  

### **Steps to Generate DPCD JSON**  
1. **Locate `drm_dp.h` from the kernel source**  
   - This file defines various DisplayPort registers and their addresses.  
   - You can find it in the kernel source tree:  
     ```
     /path/to/kernel/include/drm/drm_dp.h
     ```
   - Copy this file into the project’s `/data/` directory:  
     ```sh
     cp /path/to/kernel/include/drm/drm_dp.h data/
     ```

2. **Convert `drm_dp.h` to JSON**  
   - Use the provided script `header2json.py` to extract the register definitions.  
   - Navigate to the script’s directory:  
     ```sh
     cd scripts/
     ```
   - Run the script:  
     ```sh
     python header2json.py
     ```
   - This script parses `drm_dp.h` and outputs `dpcd_registers.json`.  

3. **Verify the JSON output**  
   - Ensure that the JSON file is correctly formatted:  
     ```sh
     cat data/dpcd_registers.json | jq .
     ```
   - The file should contain register definitions in a structured format.

---

## **Setting Up the Python Environment**  

To run the conversion scripts, a **Python virtual environment** is recommended.  

### **Steps to Set Up and Run Scripts**  
1. **Create a virtual environment**  
   ```sh
   python -m venv dev
   ```
2. **Activate the virtual environment**  
   - On Linux/macOS:  
     ```sh
     source dev/bin/activate
     ```
   - On Windows:  
     ```sh
     dev\Scripts\activate
     ```
3. **Install required dependencies**  
   ```sh
   pip install -r requirements.txt
   ```
4. **Run the scripts**  
   ```sh
   python header2json.py  # For DPCD registers
   python bxml_to_json.py  # For MMIO registers
   ```

After following these steps, **DisplayTop** will have the necessary JSON files for MMIO and DPCD register interpretation.  

For further details, refer to the [Usage Guide](usage.md).