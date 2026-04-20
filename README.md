# LottoGui

LottoGui is a small Windows desktop application for generating lottery lines and viewing simple lottery analysis results.

The program is written in C++ with Microsoft Foundation Classes (MFC) and is intended to be opened and built in Microsoft Visual Studio on Windows. The project now supports both Win32 and x64 build targets.

## Features

- Generate classic lottery lines with a custom amount of numbers and rows
- Use either a numeric range or your own input number set
- Save and print generated results
- Analyze Finnish Lotto CSV data from data/SuomenLottoData.csv
- Analyze Milli CSV data from data/MilliData.csv
- Analyze KTEM CSV data from data/KTEM.csv
- Analyze Keno CSV data from data/KenoData.csv
- Analyze Eurojackpot CSV data from data/EurojackpotData.csv
- Analyze Viking Lotto CSV data from data/VikingData.csv
- Analyze Jokeri CSV data from data/JokeriData.csv
- Create frequency-based prediction rows from historical data
- Display statistical analysis for Lotto, Milli, KTEM, Keno, Eurojackpot, Viking Lotto, and Jokeri number columns

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

## Milli CSV analysis

The application also includes a separate Milli CSV dialog in the Lotto Options menu. It reads historical Milli rows and shows:

- frequency-based prediction rows
- six main-number predictions
- the same statistical analysis used for Suomen Lotto

## KTEM CSV analysis

The application also includes a separate KTEM CSV dialog in the Lotto Options menu. It reads historical Kaikki tai ei mitään rows and shows:

- frequency-based prediction rows
- twelve main-number predictions
- the same statistical analysis used for Suomen Lotto

## Keno CSV analysis

The application also includes a separate Keno CSV dialog in the Lotto Options menu. It reads historical Keno rows and shows:

- frequency-based prediction rows
- ten predicted Keno numbers
- statistical analysis for all 20 draw columns

## Eurojackpot CSV analysis

The application also includes a separate Eurojackpot CSV dialog in the Lotto Options menu. It reads historical Eurojackpot rows and shows:

- frequency-based prediction rows
- five main-number predictions
- two star-number predictions
- the same statistical analysis used for Suomen Lotto

## Viking Lotto CSV analysis

The application also includes a separate Viking Lotto CSV dialog in the Lotto Options menu. It reads historical Viking Lotto rows and shows:

- frequency-based prediction rows
- six main-number predictions
- a Viking number added randomly from 1 to 5 when generating the prediction
- the same statistical analysis used for the other lottery modes

## Jokeri CSV analysis

The application also includes a separate Jokeri CSV dialog in the Lotto Options menu. It reads historical Jokeri rows and shows:

- frequency-based prediction rows
- seven Jokeri digit predictions
- support for repeated digits in the same row
- zero-aware statistical analysis
- harmonic mean displayed as N/A when a column contains zero values

## Data format

The CSV files should be placed in the `data` folder.

### Suomen Lotto example

The Suomen Lotto file contains semicolon-separated rows without a header line, for example:

```text
1;9;18;19;36;37;5
3;14;15;17;20;25;16
```

### Milli example

The Milli file contains semicolon-separated rows without a header line, for example:

```text
4;10;12;18;35;40
3;4;15;20;37;38
```

### KTEM example

The KTEM file contains a header line followed by semicolon-separated data rows in the format No1;No2;No3;No4;No5;No6;No7;No8;No9;No10;No11;No12.

### Keno example

The Keno file contains a header line followed by semicolon-separated data rows in the format No1;No2;...;No20.

### Eurojackpot example

The Eurojackpot file contains a header line followed by semicolon-separated data rows in the format Nr1;Nr2;Nr3;Nr4;Nr5;Star1;Star2.

### Viking Lotto example

The Viking Lotto file contains a header line followed by semicolon-separated data rows in the format Nr1;Nr2;Nr3;Nr4;Nr5;Nr6.

### Jokeri example

The Jokeri file contains a header line followed by semicolon-separated data rows in the format Nr1;Nr2;Nr3;Nr4;Nr5;Nr6;Nr7. Digits may repeat within the same row, and the value 0 is valid.

## Building the project

### Requirements

- Windows
- Microsoft Visual Studio with Desktop development with C++
- MFC support installed
- Win32 or x64 build tools available
- Inno Setup Compiler for creating a Release installer

### Build steps

1. Open the solution file LottoGui.sln in Visual Studio.
2. Select either a Win32 or x64 configuration in Debug or Release mode.
3. Build the solution.
4. Run the application.

## Creating a Release installer

An Inno Setup installer script is included at [installer/LottoGuiSetup.iss](installer/LottoGuiSetup.iss).

1. Build the x64 Release version of the application.
2. Open [installer/LottoGuiSetup.iss](installer/LottoGuiSetup.iss) in Inno Setup Compiler.
3. Compile the script.
4. The installer output is created in [installer/Output](installer/Output).

The installer packages the application executable together with all CSV files from the [data](data) folder.

## Using the application

1. Start the program.
2. Open the Lotto Options menu.
3. Choose one of the available options:
   - Classic Lotto Options
   - Suomen Lotto CSV
   - Milli CSV
   - KTEM CSV
   - Keno CSV
   - Eurojackpot CSV
   - Viking Lotto CSV
   - Jokeri CSV
4. Generate results and review them in the client area.

## Project status

This is a lightweight hobby-style MFC application focused on lottery line generation and simple statistical exploration.
