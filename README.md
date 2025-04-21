# PlotGenC++

A lightweight C++ library for generating 2D plots and charts using SFML.

## Overview
PlotGenC++ is a C++ library designed for generating 2D plots and charts. It is built on top of the SFML (Simple and Fast Multimedia Library) and provides a simple interface for creating various types of plots, including line plots, histograms, and polar plots. The library is inspired by popular plotting libraries in Python and MATLAB, making it easy to use for those familiar with those environments.

## Features

- Various **2D Charts** with multiple style options
- **Histograms** with customizable bars
- **Polar Plots** for parametric and polar functions
- **Circle Drawing** with automatic scaling
- **Text Annotations** at specific coordinates
- **Lines, Arcs and Arrows** for geometric annotations
- **Multiple layouts** to display several plots on the same figure
- **Bézier Curves** for smooth curve generation with control points
- **Splines** including natural cubic splines and cardinal splines
- **Complete customization** of colors, symbols, grids, and legends
- **PNG/JPG export** for integration into documents
- **Configurable legend positioning** including outside the plot area
- **Visual representation of styles** in legends (lines, symbols, etc.)
- **Optimized window management** (no unnecessary window display)


## Installation

### Prerequisites

- CMake 3.10 or higher
- C++ compiler with C++17 support
- SFML 2.5 or higher

## Dependencies

- [SFML](https://www.sfml-dev.org/) for graphic rendering
  - Installation: `sudo apt install libsfml-dev`
- [stb_image_write](https://github.com/nothings/stb) for image export (it is included in the repository)
  - Installation: `git clone https://github.com/nothings/stb.git`

### Build and Install

```bash
# Clone the repository
git clone https://github.com/skhelladi/PlotGenCpp.git

# Create a build directory
cd PlotGenCpp
mkdir build && cd build

# Configure and build
cmake ..
make

# Install (optional)
sudo make install
```

## Usage

### Simple Example

```cpp
#include "plotgen.h"
#include <vector>
#include <cmath>

int main() {
    PlotGen plt(800, 600);  // Create an 800x600 window
    
    // Generate data
    std::vector<float> x(100), y(100);
    for (int i = 0; i < 100; ++i) {
        x[i] = i * 0.1f - 5.0f;
        y[i] = std::sin(x[i]);
    }
    
    // Configure and plot
    auto& fig = plt.subplot(0, 0);
    PlotGen::Style style;
    style.color = sf::Color::Blue;
    style.legend = "sin(x)";
    
    plt.set_title(fig, "Sine Wave Plot");
    plt.set_xlabel(fig, "x");
    plt.set_ylabel(fig, "sin(x)");
    plt.set_axis_limits(fig, -5, 5, -1.2, 1.2);
    plt.grid(fig, true, false);
    plt.plot(fig, x, y, style);
    
    // Position the legend in the top-left corner
    plt.set_legend_position(fig, "top-left");
    
    // Add a circle and text annotation
    PlotGen::Style circle_style;
    circle_style.color = sf::Color::Red;
    circle_style.thickness = 2.0;
    plt.circle(fig, 0, 0, 1.0, circle_style);
    
    PlotGen::Style text_style;
    text_style.color = sf::Color::Green;
    plt.text(fig, 0, 1.5, "Maximum value", text_style);
    
    // Display and save
    plt.save("sinusoid.png");
    plt.show();
    
    return 0;
}
```

### Running the Examples

Run the example application to see the various available features:

```bash
./build/PlotterExamples
```

## Examples

### Example 1: Basic 2D Plots
![Basic 2D Plots](docs/example1_basic_plots.png)

### Example 2: Histograms
![Histograms](docs/example2_histograms.png)

### Example 3: Polar Plots
![Polar Plots](docs/example3_polar_plots.png)

### Example 4: Multiple Plots and Customization
![Multiple Plots](docs/example4_multiple_plots.png)

### Example 5: Advanced Histograms
![Advanced Histograms](docs/example5_advanced_histograms.png)

### Example 6: Plots with Symbols
![Plots with Symbols](docs/example6_symbol_plots.png)

### Example 7: Circles, Text and Arrows
![Circles, Text and Arrows](docs/example7_circles_text_arrows.png)

### Example 8: Bezier and Spline Curves
![Bezier and Spline Curves](docs/example8_bezier_spline.png)


## Documentation

Detailed documentation is available in the `docs/` folder.
- [Documentation in English](docs/documentation.md)
- [Documentation in French](docs/documentation_fr.md)

## License

This project is under the GPL-3.0 License. See the [LICENSE](LICENSE) file for details.

## Author

Sofiane KHELLADI

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.