# CITS3003-Project

## Setup Instructions

This project uses external libraries that are **not included in this repository**
download them from the provided `CITS3003-project.zip`

---

### Step 1: Link the lib folder

Run the following command in the project root:

```bash
ln -s /path/to/original_project/lib lib
```

### Step 2: Build the project
```bash
rm -rf build
cmake -S . -B build
cmake --build build
```

### Step 3: Run the project
```bash
./build/cits3003_project
```