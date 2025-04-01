import pdfplumber
import json
from pathlib import Path

def extract_tables_from_pdf(pdf_path):
    tables_data = []
    
    with pdfplumber.open(pdf_path) as pdf:
        for page_num, page in enumerate(pdf.pages, start=1):
            tables = page.extract_tables()
            for table_index, table in enumerate(tables, start=1):
                structured_table = [
                    {k: v.replace("\n", " ") for k, v in enumerate(row) if v is not None} 
                    for row in table
                ]
                tables_data.append({
                    "page": page_num,
                    "table_index": table_index,
                    "data": structured_table
                })
    
    return tables_data

def save_to_json(data, output_path):
    with output_path.open("w", encoding="utf-8") as json_file:
        json.dump(data, json_file, indent=4, ensure_ascii=False)

def main():
    pdf_path = Path(__file__).resolve().parent.parent / 'data' / 'dp21.pdf'
    
    json_path = Path("output.json")  # Set your JSON file name here
    
    extracted_data = extract_tables_from_pdf(pdf_path)
    save_to_json(extracted_data, json_path)
    print(f"Extracted tables saved to {json_path}")

if __name__ == "__main__":
    main()
