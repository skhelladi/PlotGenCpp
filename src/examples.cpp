#include "plotgen.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <random>
#include <functional>

// Unicode constants and symbols
const std::string DEGREE = "\u00B0";      // Degree symbol (°)
const std::string PI = "\u03C0";          // Pi symbol (π)
const std::string SQUARED = "\u00B2";     // Superscript 2 (²)
const std::string ALPHA = "\u03B1";       // Alpha (α)
const std::string BETA = "\u03B2";        // Beta (β)
const std::string THETA = "\u03B8";       // Theta (θ)
const std::string OMEGA = "\u03C9";       // Omega (ω)
const std::string DELTA = "\u0394";       // Uppercase Delta (Δ)
const std::string INF_SYMBOL = "\u221E";  // Infinity (∞)

// Example 1: Basic 2D plots
void example_basic_plots() {
    PlotGen plt(1200, 900, 2, 2);
    
    // Sinusoidal curve
    auto& fig1 = plt.subplot(0, 0);
    std::vector<double> x(100), y_sin(100);
    for (int i = 0; i < 100; ++i) {
        x[i] = i * 0.1f - 5.0f;
        y_sin[i] = std::sin(x[i]);
    }
    PlotGen::Style style_sin;
    style_sin.color = sf::Color::Blue;
    style_sin.legend = "sin(x)";
    plt.set_title(fig1, "Sine function");
    plt.set_xlabel(fig1, "x");
    plt.set_ylabel(fig1, "sin(x)");
    plt.set_axis_limits(fig1, -5, 5, -1.2, 1.2);
    plt.grid(fig1, true, false);
    plt.plot(fig1, x, y_sin, style_sin);
    // Définir la position de la légende: en haut à gauche
    plt.set_legend_position(fig1, "top-left");
    
    // Parabolic curve
    auto& fig2 = plt.subplot(0, 1);
    std::vector<double> y_parabola(100);
    for (int i = 0; i < 100; ++i) {
        y_parabola[i] = x[i] * x[i];
    }
    PlotGen::Style style_parabola;
    style_parabola.color = sf::Color::Red;
    style_parabola.legend = "f(x) = x" + SQUARED;
    plt.set_title(fig2, "Parabolic function");
    plt.set_xlabel(fig2, "x");
    plt.set_ylabel(fig2, "f(x)");
    plt.set_axis_limits(fig2, -5, 5, 0, 25);
    plt.grid(fig2, true, true);
    plt.plot(fig2, x, y_parabola, style_parabola);
    // Définir la position de la légende: en bas à droite
    plt.set_legend_position(fig2, "bottom-right");
    
    // Exponential and logarithmic curves
    auto& fig3 = plt.subplot(1, 0);
    std::vector<double> y_exp(100), y_log(100);
    for (int i = 0; i < 100; ++i) {
        y_exp[i] = std::exp(x[i] * 0.5f);
        if (x[i] > 0) y_log[i] = std::log(x[i] + 1);
        else y_log[i] = 0;
    }
    PlotGen::Style style_exp, style_log;
    style_exp.color = sf::Color::Green;
    style_exp.legend = "exp(x/2)";
    style_log.color = sf::Color::Magenta;
    style_log.legend = "ln(x+1)";
    plt.set_title(fig3, "Exponential and logarithmic functions");
    plt.set_xlabel(fig3, "x");
    plt.set_ylabel(fig3, "f(x)");
    plt.set_axis_limits(fig3, -5, 5, -1, 15);
    plt.grid(fig3, true, false);
    plt.plot(fig3, x, y_exp, style_exp);
    plt.plot(fig3, x, y_log, style_log);
    // Définir la position de la légende: en bas à gauche
    plt.set_legend_position(fig3, "bottom-left");
    plt.text(fig3, 1.0, 1.0, "Logarithmic Function", PlotGen::Style(sf::Color::Red));
    
    // Different line styles
    auto& fig4 = plt.subplot(1, 1);
    std::vector<double> y_cos(100), y_tan(100);
    for (int i = 0; i < 100; ++i) {
        y_cos[i] = std::cos(x[i]);
        // Avoid infinite tangent values
        if (std::abs(std::cos(x[i])) > 0.1) {
            y_tan[i] = std::tan(x[i]);
            // Limit amplitude for visualization
            if (y_tan[i] > 5) y_tan[i] = 5;
            if (y_tan[i] < -5) y_tan[i] = -5;
        } else {
            y_tan[i] = 0;
        }
    }
    PlotGen::Style style_cos, style_tan;
    style_cos.color = sf::Color::Cyan;
    style_cos.line_style = "dashed";
    style_cos.legend = "cos(x) [dashed]";
    style_tan.color = sf::Color(255, 165, 0); // Orange
    style_tan.line_style = "points";
    style_tan.legend = "tan(x) [points]";
    plt.set_title(fig4, "Different line styles");
    plt.set_xlabel(fig4, "x");
    plt.set_ylabel(fig4, "f(x)");
    plt.set_axis_limits(fig4, -5, 5, -5, 5);
    plt.grid(fig4, true, false);
    plt.plot(fig4, x, y_cos, style_cos);
    plt.plot(fig4, x, y_tan, style_tan);
    // Définir la position de la légende: à l'extérieur à droite
    plt.set_legend_position(fig4, "outside-right");
    
    plt.save("example1_basic_plots.png");
    plt.show();
}

// Example 2: Histograms
void example_histograms() {
    PlotGen plt(1200, 900, 2, 2);
    
    // Normal distribution
    std::default_random_engine generator;
    std::normal_distribution<double> normal_dist(0.0, 1.0);
    std::vector<double> normal_data(1000);
    for (int i = 0; i < 1000; ++i) {
        normal_data[i] = normal_dist(generator);
    }
    
    // Histogram with different numbers of bins
    auto& fig1 = plt.subplot(0, 0);
    PlotGen::Style style1;
    style1.color = sf::Color::Blue;
    style1.legend = "10 bins";
    plt.set_title(fig1, "Normal distribution (10 bins)");
    plt.set_xlabel(fig1, "Value");
    plt.set_ylabel(fig1, "Frequency");
    // plt.set_axis_limits(fig1, -5, 5, 0, 300);
    plt.hist(fig1, normal_data, 10, style1);
    
    auto& fig2 = plt.subplot(0, 1);
    PlotGen::Style style2;
    style2.color = sf::Color::Red;
    style2.legend = "30 bins";
    plt.set_title(fig2, "Normal distribution (30 bins)");
    plt.set_xlabel(fig2, "Value");
    plt.set_ylabel(fig2, "Frequency");
    // plt.set_axis_limits(fig2, -5.0, 5.0, 0, 300);
    plt.hist(fig2, normal_data, 30, style2);
    
    // Uniform distribution
    std::uniform_real_distribution<double> uniform_dist(0.0, 10.0);
    std::vector<double> uniform_data(1000);
    for (int i = 0; i < 1000; ++i) {
        uniform_data[i] = uniform_dist(generator);
    }
    
    auto& fig3 = plt.subplot(1, 0);
    PlotGen::Style style3;
    style3.color = sf::Color::Green;
    style3.legend = "Uniform distribution";
    plt.set_title(fig3, "Uniform distribution");
    plt.set_xlabel(fig3, "Value");
    plt.set_ylabel(fig3, "Frequency");
    // plt.set_axis_limits(fig3, -5.0, 5.0, 0, 300);
    plt.hist(fig3, uniform_data, 20, style3);
    
    // Exponential distribution
    std::exponential_distribution<double> exp_dist(0.5);
    std::vector<double> exp_data(1000);
    for (int i = 0; i < 1000; ++i) {
        exp_data[i] = exp_dist(generator);
    }
    
    auto& fig4 = plt.subplot(1, 1);
    PlotGen::Style style4;
    style4.color = sf::Color::Magenta;
    style4.legend = "Exponential distribution";
    plt.set_title(fig4, "Exponential distribution");
    plt.set_xlabel(fig4, "Value");
    plt.set_ylabel(fig4, "Frequency");
    // plt.set_axis_limits(fig4, -5.0, 5.0, 0, 400);
    plt.hist(fig4, exp_data, 25, style4);
    
    plt.save("example2_histograms.png");
    plt.show();
}

// Example 3: Polar plots
void example_polar_plots() {
    PlotGen plt(1200, 900, 2, 2);
    
    // 4-petal rose
    auto& fig1 = plt.subplot(0, 0);
    std::vector<double> theta1(500), r1(500);
    for (int i = 0; i < 500; ++i) {
        theta1[i] = i * 0.05f;
        r1[i] = std::abs(std::cos(2 * theta1[i]));
    }
    PlotGen::Style style1;
    style1.color = sf::Color::Red;
    style1.thickness = 2.5f;
    style1.legend = "r = |cos(2" + THETA + ")|";
    plt.set_title(fig1, "4-petal rose");
    plt.set_xlabel(fig1, "X axis (" + DEGREE + ")");
    plt.set_ylabel(fig1, "Y axis (" + DEGREE + ")");
    // plt.set_axis_limits(fig1, -1.2, 1.2, -1.2, 1.2);
    plt.grid(fig1, true, true);
    plt.polar_plot(fig1, theta1, r1, style1);
    
    // Cardioid
    auto& fig2 = plt.subplot(0, 1);
    std::vector<double> theta2(300), r2(300);
    for (int i = 0; i < 300; ++i) {
        theta2[i] = i * 0.02f * 3.14159f;
        r2[i] = 1 + std::cos(theta2[i]);
    }
    PlotGen::Style style2;
    style2.color = sf::Color::Blue;
    style2.thickness = 2.5f;
    style2.legend = "r = 1 + cos(" + THETA + ")";
    plt.set_title(fig2, "Cardioid");
    plt.set_xlabel(fig2, "x = r·cos(" + THETA + ")");
    plt.set_ylabel(fig2, "y = r·sin(" + THETA + ")");
    // plt.set_axis_limits(fig2, -2.5, 2.5, -2.5, 2.5);
    plt.grid(fig2, true, false);
    plt.polar_plot(fig2, theta2, r2, style2);
    
    // Archimedean spiral
    auto& fig3 = plt.subplot(1, 0);
    std::vector<double> theta3(200), r3(200);
    for (int i = 0; i < 200; ++i) {
        theta3[i] = i * 0.1f;
        r3[i] = 0.2f * theta3[i];
    }
    PlotGen::Style style3;
    style3.color = sf::Color::Green;
    style3.thickness = 2.0f;
    style3.legend = "r = 0.2" + THETA;
    plt.set_title(fig3, "Archimedean spiral");
    plt.set_xlabel(fig3, "X axis");
    plt.set_ylabel(fig3, "Y axis");
    plt.grid(fig3, true, true);
    plt.polar_plot(fig3, theta3, r3, style3);
    
    // Limacon of Pascal
    auto& fig4 = plt.subplot(1, 1);
    std::vector<double> theta4(300), r4(300);
    for (int i = 0; i < 300; ++i) {
        theta4[i] = i * 0.02f * 3.14159f;
        r4[i] = 0.5f + std::cos(theta4[i]);
    }
    PlotGen::Style style4;
    style4.color = sf::Color::Magenta;
    style4.thickness = 5.0f;
    style4.legend = "r = 0.5 + cos(" + THETA + ")";
    plt.set_title(fig4, "Limacon of Pascal");
    plt.set_xlabel(fig4, "X axis");
    plt.set_ylabel(fig4, "Y axis");
    plt.grid(fig4, true, false);
    plt.polar_plot(fig4, theta4, r4, style4);
    
    plt.save("example3_polar_plots.png");
    plt.show();
}

// Example 4: Multiple plots and customization
void example_multiple_plots() {
    PlotGen plt(1200, 900, 2, 2);
    
    // Multiple curves on the same plot
    auto& fig1 = plt.subplot(0, 0);
    std::vector<double> x(100);
    std::vector<double> y1(100), y2(100), y3(100);
    for (int i = 0; i < 100; ++i) {
        x[i] = i * 0.1f - 5.0f;
        y1[i] = std::sin(x[i]);
        y2[i] = std::cos(x[i]);
        y3[i] = 0.5f * std::sin(2 * x[i]);
    }
    
    PlotGen::Style style1, style2, style3;
    style1.color = sf::Color::Blue;
    style1.legend = "sin(x)";
    style2.color = sf::Color::Red;
    style2.legend = "cos(x)";
    style3.color = sf::Color::Green;
    style3.legend = "0.5·sin(2x)";
    
    plt.set_title(fig1, "Trigonometric functions");
    plt.set_xlabel(fig1, "x");
    plt.set_ylabel(fig1, "f(x)");
    plt.set_axis_limits(fig1, -5, 5, -1.2, 1.2);
    plt.grid(fig1, true, false);
    plt.plot(fig1, x, y1, style1);
    plt.plot(fig1, x, y2, style2);
    plt.plot(fig1, x, y3, style3);
    
    // Equal axes for a perfect circle
    auto& fig2 = plt.subplot(0, 1);
    std::vector<double> circle_x(100), circle_y(100);
    for (int i = 0; i < 100; ++i) {
        double angle = 2 * 3.14159f * i / 99;
        circle_x[i] = std::cos(angle);
        circle_y[i] = std::sin(angle);
    }
    
    PlotGen::Style style_circle;
    style_circle.color = sf::Color::Blue;
    style_circle.thickness = 3.0f;
    style_circle.legend = "Unit circle";
    
    plt.set_title(fig2, "Circle with equal axes");
    plt.set_xlabel(fig2, "x");
    plt.set_ylabel(fig2, "y");
    plt.set_axis_limits(fig2, -1.5, 1.5, -1.5, 1.5);
    plt.grid(fig2, true, true);
    plt.set_equal_axes(fig2, true); // Enable equal axes
    plt.plot(fig2, circle_x, circle_y, style_circle);
    
    // Ellipse with unequal axes
    auto& fig3 = plt.subplot(1, 0);
    std::vector<double> ellipse_x(100), ellipse_y(100);
    for (int i = 0; i < 100; ++i) {
        double angle = 2 * 3.14159f * i / 99;
        ellipse_x[i] = 2 * std::cos(angle);
        ellipse_y[i] = std::sin(angle);
    }
    
    PlotGen::Style style_ellipse;
    style_ellipse.color = sf::Color::Red;
    style_ellipse.thickness = 3.0f;
    style_ellipse.legend = "Ellipse 2:1";
    
    plt.set_title(fig3, "Ellipse without equal axes");
    plt.set_xlabel(fig3, "x");
    plt.set_ylabel(fig3, "y");
    plt.set_axis_limits(fig3, -2.5, 2.5, -1.5, 1.5);
    plt.grid(fig3, true, true);
    // Do not enable equal axes
    plt.plot(fig3, ellipse_x, ellipse_y, style_ellipse);
    
    // Custom grid colors
    auto& fig4 = plt.subplot(1, 1);
    std::vector<double> lissajous_x(1000), lissajous_y(1000);
    for (int i = 0; i < 1000; ++i) {
        double t = i * 0.01f;
        lissajous_x[i] = std::sin(3 * t);
        lissajous_y[i] = std::sin(2 * t);
    }
    
    PlotGen::Style style_lissajous;
    style_lissajous.color = sf::Color::Green;
    style_lissajous.thickness = 2.0f;
    style_lissajous.legend = "Lissajous curve";
    
    plt.set_title(fig4, "Lissajous curve with colored grid");
    plt.set_xlabel(fig4, "sin(3t)");
    plt.set_ylabel(fig4, "sin(2t)");
    plt.set_axis_limits(fig4, -1.5, 1.5, -1.5, 1.5);
    plt.grid(fig4, true, true);
    plt.set_grid_color(fig4, sf::Color(100, 100, 200), sf::Color(200, 200, 255));
    plt.set_equal_axes(fig4, true); // Enable equal axes
    plt.plot(fig4, lissajous_x, lissajous_y, style_lissajous);
    
    plt.save("example4_multiple_plots.png");
    plt.show();
}

// Example 5: Advanced histograms (with variable colors and thicknesses)
void example_advanced_histograms() {
    PlotGen plt(1200, 900, 2, 2);
    
    // Random number generator
    std::default_random_engine generator;
    
    // First distribution: Normal with 2 modes
    std::vector<double> bimodal_data(1000);
    std::normal_distribution<double> dist1(-3.0, 1.0);
    std::normal_distribution<double> dist2(3.0, 1.0);
    for (int i = 0; i < 1000; ++i) {
        if (i < 500) {
            bimodal_data[i] = dist1(generator);
        } else {
            bimodal_data[i] = dist2(generator);
        }
    }
    
    // Bimodal histogram with thin bars
    auto& fig1 = plt.subplot(0, 0);
    PlotGen::Style style1;
    style1.color = sf::Color(75, 0, 130); // Indigo
    style1.legend = "Bimodal distribution with thin bars";
    plt.set_title(fig1, "Bimodal distribution (thin bars)");
    plt.set_xlabel(fig1, "Value");
    plt.set_ylabel(fig1, "Frequency");
    plt.hist(fig1, bimodal_data, 40, style1, 0.5f); // Thin bars (50% width)
    
    // Same distribution with wider bars
    auto& fig2 = plt.subplot(0, 1);
    PlotGen::Style style2;
    style2.color = sf::Color(148, 0, 211); // Violet
    style2.legend = "Bimodal distribution with wide bars";
    plt.set_title(fig2, "Bimodal distribution (wide bars)");
    plt.set_xlabel(fig2, "Value");
    plt.set_ylabel(fig2, "Frequency");
    plt.hist(fig2, bimodal_data, 40, style2, 1.0f); // Full bars (100% width)
    
    // Exponential distribution with different colors
    std::vector<double> exp_data1(500);
    std::vector<double> exp_data2(500);
    std::exponential_distribution<double> exp_dist1(0.5);
    std::exponential_distribution<double> exp_dist2(1.0);
    for (int i = 0; i < 500; ++i) {
        exp_data1[i] = exp_dist1(generator);
        exp_data2[i] = exp_dist2(generator); 
    }
    
    // Histogram with two distributions on the same plot
    auto& fig3 = plt.subplot(1, 0);
    PlotGen::Style style3a, style3b;
    style3a.color = sf::Color(220, 20, 60); // Crimson
    style3b.color = sf::Color(255, 140, 0); // Dark orange
    style3a.legend = "Lambda = 0.5";
    style3b.legend = "Lambda = 1.0";
    plt.set_title(fig3, "Comparison of exponential distributions");
    plt.set_xlabel(fig3, "Value");
    plt.set_ylabel(fig3, "Frequency");
    plt.hist(fig3, exp_data1, 25, style3a, 0.8f);
    plt.hist(fig3, exp_data2, 25, style3b, 0.4f); // Thinner to be visible
    
    // Custom distribution - chi-squared distribution
    std::vector<double> chi2_data(1000);
    for (int i = 0; i < 1000; ++i) {
        // Simulate Chi-squared with k=3 degrees of freedom
        double sum = 0;
        for (int j = 0; j < 3; ++j) {
            std::normal_distribution<double> normal(0, 1);
            double x = normal(generator);
            sum += x * x;
        }
        chi2_data[i] = sum;
    }
    
    // Histogram with color gradient (multiple distributions)
    auto& fig4 = plt.subplot(1, 1);
    
    // Create three styles with different colors
    std::vector<PlotGen::Style> styles;
    std::vector<double> width_ratios = {0.7f, 0.7f, 0.7f};
    
    PlotGen::Style style4a;
    style4a.color = sf::Color(30, 144, 255); // Dodger blue
    style4a.legend = "Chi-squared (low values)";
    styles.push_back(style4a);
    
    PlotGen::Style style4b;
    style4b.color = sf::Color(60, 179, 113); // Medium sea green
    style4b.legend = "Chi-squared (medium values)";
    styles.push_back(style4b);
    
    PlotGen::Style style4c;
    style4c.color = sf::Color(255, 69, 0); // Orange red
    style4c.legend = "Chi-squared (high values)";
    styles.push_back(style4c);
    
    // Divide the data into three parts
    double min_val = *std::min_element(chi2_data.begin(), chi2_data.end());
    double max_val = *std::max_element(chi2_data.begin(), chi2_data.end());
    double range = max_val - min_val;
    double third = range / 3;
    
    std::vector<double> chi2_part1, chi2_part2, chi2_part3;
    
    for (double val : chi2_data) {
        if (val < min_val + third) {
            chi2_part1.push_back(val);
        } else if (val < min_val + 2 * third) {
            chi2_part2.push_back(val);
        } else {
            chi2_part3.push_back(val);
        }
    }
    
    plt.set_title(fig4, "Chi-squared distribution (k=3) by ranges");
    plt.set_xlabel(fig4, "Value");
    plt.set_ylabel(fig4, "Frequency");
    
    // Plot the histograms with different colors
    plt.hist(fig4, chi2_part1, 10, styles[0], width_ratios[0]);
    plt.hist(fig4, chi2_part2, 10, styles[1], width_ratios[1]);
    plt.hist(fig4, chi2_part3, 10, styles[2], width_ratios[2]);
    
    plt.save("example5_advanced_histograms.png");
    plt.show();
}

// Example 6: Curves with symbols
void example_symbol_plots() {
    PlotGen plt(1200, 900, 2, 2);
    
    // Example 1: Different types of symbols
    auto& fig1 = plt.subplot(0, 0);
    std::vector<double> x(10), y1(10), y2(10), y3(10), y4(10), y5(10);
    
    for (int i = 0; i < 10; ++i) {
        x[i] = i;
        y1[i] = std::sin(x[i] * 0.4f);
        y2[i] = std::sin(x[i] * 0.4f) + 0.5f;
        y3[i] = std::sin(x[i] * 0.4f) + 1.0f;
        y4[i] = std::sin(x[i] * 0.4f) + 1.5f;
        y5[i] = std::sin(x[i] * 0.4f) + 2.0f;
    }
    
    // Different symbols
    PlotGen::Style style_circle, style_square, style_triangle, style_diamond, style_star;
    
    style_circle.color = sf::Color::Red;
    style_circle.symbol_type = "circle";
    style_circle.symbol_size = 8.0f;
    style_circle.line_style = "none"; // No line, only symbols
    style_circle.legend = "Circle";
    
    style_square.color = sf::Color::Blue;
    style_square.symbol_type = "square";
    style_square.symbol_size = 8.0f;
    style_square.line_style = "none";
    style_square.legend = "Square";
    
    style_triangle.color = sf::Color::Green;
    style_triangle.symbol_type = "triangle";
    style_triangle.symbol_size = 8.0f;
    style_triangle.line_style = "none";
    style_triangle.legend = "Triangle";
    
    style_diamond.color = sf::Color::Yellow;
    style_diamond.symbol_type = "diamond";
    style_diamond.symbol_size = 8.0f;
    style_diamond.line_style = "none";
    style_diamond.legend = "Diamond";
    
    style_star.color = sf::Color::Magenta;
    style_star.symbol_type = "star";
    style_star.symbol_size = 8.0f;
    style_star.line_style = "none";
    style_star.legend = "Star";
    
    plt.set_title(fig1, "Types of symbols");
    plt.set_xlabel(fig1, "X");
    plt.set_ylabel(fig1, "Y");
    plt.set_axis_limits(fig1, -1, 10, -1.5, 3.0);
    plt.grid(fig1, true, false);
    
    plt.plot(fig1, x, y1, style_circle);
    plt.plot(fig1, x, y2, style_square);
    plt.plot(fig1, x, y3, style_triangle);
    plt.plot(fig1, x, y4, style_diamond);
    plt.plot(fig1, x, y5, style_star);
    
    // Example 2: Different symbol sizes
    auto& fig2 = plt.subplot(0, 1);
    
    std::vector<double> x2(10), y_sizes(5);
    for (int i = 0; i < 10; ++i) {
        x2[i] = i;
    }
    
    for (int j = 0; j < 5; ++j) {
        y_sizes[j] = j * 0.5f;
    }
    
    std::vector<PlotGen::Style> size_styles;
    std::vector<double> sizes = {4.0f, 8.0f, 12.0f, 16.0f, 20.0f};
    
    for (int j = 0; j < 5; ++j) {
        std::vector<double> y_curve(10);
        for (int i = 0; i < 10; ++i) {
            y_curve[i] = y_sizes[j];
        }
        
        PlotGen::Style style;
        style.color = sf::Color(50 + j * 50, 100, 200);
        style.symbol_type = "circle";
        style.symbol_size = sizes[j];
        style.line_style = "none";
        style.legend = "Size " + std::to_string(static_cast<int>(sizes[j]));
        
        plt.plot(fig2, x2, y_curve, style);
    }
    
    plt.set_title(fig2, "Symbol sizes");
    plt.set_xlabel(fig2, "X");
    plt.set_ylabel(fig2, "Y");
    plt.set_axis_limits(fig2, -1, 10, -0.5, 2.5);
    plt.grid(fig2, true, false);
    
    // Example 3: Combination of lines and symbols
    auto& fig3 = plt.subplot(1, 0);
    
    std::vector<double> x3(100), y_sin(100), y_cos(100);
    for (int i = 0; i < 100; ++i) {
        x3[i] = i * 0.1f;
        y_sin[i] = std::sin(x3[i]);
        y_cos[i] = std::cos(x3[i]);
    }
    
    PlotGen::Style style_sin_line, style_sin_circle, style_cos_line, style_cos_square;
    
    // Style for sin(x) with a solid line
    style_sin_line.color = sf::Color::Blue;
    style_sin_line.thickness = 2.0f;
    style_sin_line.line_style = "solid";
    style_sin_line.legend = "sin(x) - line";
    
    // Style for sin(x) with circles
    style_sin_circle.color = sf::Color::Blue;
    style_sin_circle.symbol_type = "circle";
    style_sin_circle.symbol_size = 6.0f;
    style_sin_circle.line_style = "none";
    style_sin_circle.legend = "sin(x) - points";
    
    // Style for cos(x) with a solid line
    style_cos_line.color = sf::Color::Red;
    style_cos_line.thickness = 2.0f;
    style_cos_line.line_style = "solid";
    style_cos_line.legend = "cos(x) - line";
    
    // Style for cos(x) with squares
    style_cos_square.color = sf::Color::Red;
    style_cos_square.symbol_type = "square";
    style_cos_square.symbol_size = 6.0f;
    style_cos_square.line_style = "none";
    style_cos_square.legend = "cos(x) - points";
    
    plt.set_title(fig3, "Combination of lines and symbols");
    plt.set_xlabel(fig3, "X");
    plt.set_ylabel(fig3, "Y");
    plt.set_axis_limits(fig3, 0, 10, -1.5, 1.5);
    plt.grid(fig3, true, false);
    
    // Plot only a few points for the symbols
    std::vector<double> x_points(10), y_sin_points(10), y_cos_points(10);
    for (int i = 0; i < 10; ++i) {
        int idx = i * 10;
        x_points[i] = x3[idx];
        y_sin_points[i] = y_sin[idx];
        y_cos_points[i] = y_cos[idx];
    }
    
    plt.plot(fig3, x3, y_sin, style_sin_line);
    plt.plot(fig3, x_points, y_sin_points, style_sin_circle);
    plt.plot(fig3, x3, y_cos, style_cos_line);
    plt.plot(fig3, x_points, y_cos_points, style_cos_square);
    
    // Example 4: Line with integrated symbols
    auto& fig4 = plt.subplot(1, 1);
    
    // Data for a quadratic curve
    std::vector<double> x4(100), y_quad(100);
    for (int i = 0; i < 100; ++i) {
        x4[i] = i * 0.1f;
        y_quad[i] = 0.05f * x4[i] * x4[i];
    }
    
    // Style for the line with triangles
    PlotGen::Style style_quad_line_symbols;
    style_quad_line_symbols.color = sf::Color::Green;
    style_quad_line_symbols.thickness = 2.0f;
    style_quad_line_symbols.line_style = "solid";
    style_quad_line_symbols.symbol_type = "diamond";  // Use diamonds
    style_quad_line_symbols.symbol_size = 8.0f;
    // style_quad_line_symbols.symbol_freq = 10;         // Show a symbol every 10 points
    style_quad_line_symbols.legend = "f(x) = 0.05x² with symbols";
    
    plt.set_title(fig4, "Line with integrated symbols");
    plt.set_xlabel(fig4, "X");
    plt.set_ylabel(fig4, "Y");
    plt.set_axis_limits(fig4, 0, 10, -0.5, 5.5);
    plt.grid(fig4, true, false);
    
    plt.plot(fig4, x4, y_quad, style_quad_line_symbols);
    // plt.circle(fig4, 5.0f, 2.5f, 0.5f, sf::Color::Red); // Draw a circle at (5, 2.5)
    
    plt.save("example6_symbol_plots.png");
    plt.show();
}

// Main program to choose which example to run
int main() {
    std::cout << "PlotGen - Plotting examples" << std::endl;
    std::cout << "1. Basic 2D plots" << std::endl;
    std::cout << "2. Histograms" << std::endl;
    std::cout << "3. Polar plots" << std::endl;
    std::cout << "4. Multiple plots and customization" << std::endl;
    std::cout << "5. Advanced histograms" << std::endl;
    std::cout << "6. Curves with symbols" << std::endl;
    std::cout << "Enter your choice (1-6): ";
    
    int choice;
    std::cin >> choice;
    
    switch (choice) {
        case 1: example_basic_plots(); break;
        case 2: example_histograms(); break;
        case 3: example_polar_plots(); break;
        case 4: example_multiple_plots(); break;
        case 5: example_advanced_histograms(); break;
        case 6: example_symbol_plots(); break;
        default: 
            std::cout << "Invalid choice." << std::endl;
            return 1;
    }
    
    return 0;
}