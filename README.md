# LottoGui

LottoGui is a small Windows desktop application for generating lottery lines and viewing simple lottery analysis results.

The program is written in C++ with Microsoft Foundation Classes (MFC) and is intended to be opened and built in Microsoft Visual Studio on Windows. The project now supports both Win32 and x64 build targets.

## Features

- Generate classic lottery lines with a custom amount of numbers and rows
- Use either a numeric range or your own input number set
- Save and print generated results
- Analyze Finnish Lotto CSV data from `data/SuomenLottoData.csv`
- Create frequency-based prediction rows from historical data
- Display statistical analysis for columns Nr1-Nr7

## Finnish Lotto CSV analysis

The application includes a separate Suomen Lotto dialog available from the Lotto Options menu. It reads the historical CSV file and shows:

- prediction rows based on historical frequency
- mean
- minimum and maximum
- median
- mode
- harmonic mean
- geometric mean
- root mean square
- standard deviation
- lower and upper quartiles

> Note: the predictions are statistical estimates based on past data and are not guarantees of future results.

## Data format

The CSV file should be placed in the `data` folder and should contain rows of semicolon-separated Finnish Lotto main numbers without a header line, for example:

```text
1;9;18;19;36;37;5
3;14;15;17;20;25;16
```

## Building the project

### Requirements

- Windows
- Microsoft Visual Studio with Desktop development with C++
- MFC support installed
- Win32 or x64 build tools available

### Build steps

1. Open the solution file `LottoGui.sln` in Visual Studio.
2. Select either a Win32 or x64 configuration in Debug or Release mode.
3. Build the solution.
4. Run the application.

## Using the application

1. Start the program.
2. Open the Lotto Options menu.
3. Choose either:
   - Classic Lotto Options
   - Suomen Lotto CSV
4. Generate results and review them in the client area.

## Project status

This is a lightweight hobby-style MFC application focused on lottery line generation and simple statistical exploration.
