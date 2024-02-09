# RandomTrendify

RandomTrendify is a sophisticated tool designed to generate market trend curves, identify peaks and troughs within those trends, and apply these insights along with randomization to datasets. Built with a focus on financial markets and data analysis, RandomTrendify offers a unique blend of trend analysis and data manipulation capabilities, making it an invaluable tool for economists, data scientists, and market analysts.

## Features

- **Market Trend Generation**: Create synthetic market trend curves based on specified parameters, offering insights into potential market movements.
- **Peak and Trough Localization**: Automatically identify and segment peaks and troughs within the generated market trends, allowing for detailed trend analysis.
- **Data Randomization**: Apply a layer of randomization to datasets, simulating the unpredictability of real-world market conditions.
- **Customizable Data Processing**: Process and manipulate input data files, applying market trends and randomization algorithms to generate output datasets.
- **Graphical Visualization**: Utilize a graphical interface to visualize the generated market trends and the effects of applied randomization.

## Installation

RandomTrendify is built using C++ and relies on the wxWidgets library for its graphical user interface. To install RandomTrendify, follow these steps:

1. **Clone the Repository**: ```git clone https://github.com/alexkat/RandomTrendify.git```
2. **Install wxWidgets**:
Ensure wxWidgets is installed on your system. Visit [wxWidgets' official documentation](https://docs.wxwidgets.org/) for installation instructions.
3. **Compile the Application**:
Navigate to the RandomTrendify directory and compile the application using your preferred C++ compiler. For example, using g++:
    ```g++ -o RandomTrendify *.cpp wx-config --cxxflags --libs -std=c++11```

## Usage

To use RandomTrendify, run the compiled application. The GUI will guide you through the process of opening a data file, generating market trends, and applying randomization.

### Example Workflow

1. **Open File**: Click on "Open File" to select your input JSON data file.
2. **Generate Trends**: Click on "Generate" to apply market trend analysis and randomization to your data.
3. **Save File**: After generating the output, click on "Save" to save the modified dataset.

## Contributing

Contributions to RandomTrendify are welcome! If you have suggestions for improvements or new features, feel free to open an issue or submit a pull request.

### Contributing Guidelines

- **Code Style**: Please adhere to the existing code style for consistency.
- **Feature Requests**: Open an issue to discuss potential features before starting work.
- **Bug Reports**: If you find a bug, please open an issue with a clear description and steps to reproduce.

## License

RandomTrendify is open-source software licensed under the MIT License. See the LICENSE file for more details.

## Acknowledgments

- Thanks to the wxWidgets team for their excellent GUI library.
- Special thanks to all contributors and users of RandomTrendify.

For more information, questions, or feedback, please contact the project maintainers.

