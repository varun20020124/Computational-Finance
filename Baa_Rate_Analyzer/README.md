# Baa Rate Analyzer

## Overview

The Baa Rate Analyzer is a C++ command-line program that reads historical Moody's Baa corporate bond rate data from a CSV file and allows a user to query the rate associated with a particular month.

The user provides a date in `yyyy-mm` format. The program searches the historical dataset for that month and, when the date exists, reports:

1. The Baa rate for the requested month.
2. The difference between that rate and the average Baa rate across the complete dataset.

If the requested date does not exist in the dataset, the program displays an informative message rather than terminating or crashing.

The user may continue entering dates until an EOF signal is provided:

* `Ctrl-D` on macOS/Linux
* `Ctrl-Z` on Windows

## Project Structure

```text
Baa_Rate_Analyzer/
├── homework_1.cpp
├── hw1_H.15_Baa_Data.csv
├── README.md
└── screenshots/
```

### `homework_1.cpp`

Contains the C++ implementation of the Baa rate analyzer.

### `hw1_H.15_Baa_Data.csv`

Contains the historical Baa rate data used by the program.

The data file is read directly by the C++ program and is not manually modified. Metadata and header rows in the CSV are handled programmatically during file parsing.

### `screenshots/`

Contains screenshots demonstrating compilation and successful execution of the program.

---

## Program Components

### `average`

```cpp
double average(vector<double> v)
```

Calculates the arithmetic mean of all rates stored in the supplied vector.

The function:

* checks whether the vector is empty,
* sums all rate values,
* divides the total by the number of observations,
* returns the resulting average as a `double`.

---

### `find_rate`

```cpp
double find_rate(
    vector<double> rate_vec,
    vector<string> date_vec,
    string date
)
```

Searches the date vector for a requested date in `yyyy-mm` format.

The `date_vec` and `rate_vec` vectors are parallel vectors, meaning that a date and its corresponding rate occupy the same index.

For example:

```text
date_vec[i]  -> requested month
rate_vec[i]  -> rate for that month
```

When the date is found, the corresponding rate is returned.

If the date is not present in the dataset, the function returns `-1.0`, which is used by the main program as a sentinel value indicating that no matching observation was found.

---

### CSV Loading

The program opens:

```text
hw1_H.15_Baa_Data.csv
```

using an `ifstream`.

The CSV contains metadata and header information before the historical observations. The program reads through these rows until it reaches the data header containing `Time Period`.

Each subsequent data row is separated into:

```text
date,rate
```

The date is stored in:

```cpp
vector<string> date;
```

and the corresponding rate is converted from text to a `double` and stored in:

```cpp
vector<double> rate;
```

The original CSV file does not need to be edited for the program to run.

---

### User Input

After loading the dataset, the program calculates the historical average and prompts the user for a date:

```text
Enter a date in yyyy-mm format (Ctrl-D to exit):
```

Input continues inside a loop until the user sends an EOF signal.

For a valid date, the program displays:

```text
Rate: <rate>
Difference from average: <difference>
```

For a date that does not appear in the dataset, the program displays:

```text
The input date is not in the data.
```

The program then continues accepting additional dates.

---

## Requirements

A C++17-compatible compiler is required.

The project has been developed and tested using Apple Clang on macOS.

No external C++ libraries are required.

---

## Compile

Open a terminal and navigate to the project directory:

```bash
cd Baa_Rate_Analyzer
```

Compile the program using:

```bash
clang++ -std=c++17 -Wall -Wextra homework_1.cpp -o homework_1
```

If compilation is successful, an executable named:

```text
homework_1
```

will be created.

---

## Run

From the same directory, run:

```bash
./homework_1
```

The program will prompt for a date.

Example:

```text
Enter a date in yyyy-mm format (Ctrl-D to exit):
2002-01
```

The program will either display the corresponding rate and its difference from the historical average or report that the date does not exist in the dataset.

Additional dates may then be entered.

To terminate the program on macOS/Linux, press:

```text
Ctrl-D
```

---

## Compile and Run in One Command

For convenience:

```bash
clang++ -std=c++17 -Wall -Wextra homework_1.cpp -o homework_1 && ./homework_1
```

The executable is run only if compilation succeeds.

---

## Error Handling

The implementation includes checks for several possible problems:

* The CSV file cannot be opened.
* The dataset contains empty or malformed rows.
* No valid observations are loaded.
* The user requests a date that is not present in the dataset.
* The rate vector supplied to `average` is empty.

Invalid date queries do not terminate the program. The user may continue entering other dates.

---

## Data Integrity

The supplied CSV data file is treated as read-only input.

No manual changes to the source dataset are required or performed by the program. Metadata and header rows are handled directly by the C++ parsing logic.

---

## Author

Varun Jhaveri

Georgia Institute of Technology
ISYE 6767
