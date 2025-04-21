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

// Example 7: Circles, Text, Arrows, Lines and Arcs
void example_circles_text_arrows() {
    PlotGen plt(1200, 900, 2, 2);
    
    // Example 1: Circles with different styles and lines
    auto& fig1 = plt.subplot(0, 0);
    
    // Create different styles for the circles and lines
    PlotGen::Style style_circle1, style_circle2, style_circle3;
    PlotGen::Style style_line1, style_line2;
    
    style_circle1.color = sf::Color::Red;
    style_circle1.thickness = 2.0;
    style_circle1.legend = "Circle (3,2) r=1";
    
    style_circle2.color = sf::Color::Blue;
    style_circle2.thickness = 3.0;
    style_circle2.legend = "Circle (0,0) r=3";
    
    style_circle3.color = sf::Color::Green;
    style_circle3.thickness = 1.0;
    style_circle3.line_style = "dashed";
    style_circle3.legend = "Dashed circle (1,-2) r=2";
    
    style_line1.color = sf::Color::Magenta;
    style_line1.thickness = 2.5;
    style_line1.legend = "Diagonal line";
    
    style_line2.color = sf::Color::Cyan;
    style_line2.thickness = 1.5;
    style_line2.line_style = "dashed";
    style_line2.legend = "Dashed horizontal line";
    
    plt.set_title(fig1, "Circles and lines");
    plt.set_xlabel(fig1, "X");
    plt.set_ylabel(fig1, "Y");
    plt.grid(fig1, true, false);
    
    // Draw three circles with different positions, sizes and styles
    plt.circle(fig1, 3, 2, 1, style_circle1);     // Red circle at (3,2) with radius 1
    plt.circle(fig1, 0, 0, 3, style_circle2);     // Blue circle at (0,0) with radius 3
    plt.circle(fig1, 1, -2, 2, style_circle3);    // Green dashed circle at (1,-2) with radius 2
    
    // Add lines to connect circles
    plt.line(fig1, 3, 2, 0, 0, style_line1);       // Diagonal line connecting two circles
    plt.line(fig1, -3, -2, 3, -2, style_line2);    // Horizontal dashed line
    
    // Example 2: Text annotations and arcs
    auto& fig2 = plt.subplot(0, 1);
    
    // Generate some data for a parabola
    std::vector<double> x(100), y(100);
    for (int i = 0; i < 100; ++i) {
        x[i] = i * 0.1 - 5.0;
        y[i] = x[i] * x[i];
    }
    
    // Create a style for the parabola, text annotations and arcs
    PlotGen::Style style_curve, style_text1, style_text2, style_text3;
    PlotGen::Style style_arc1, style_arc2;
    
    style_curve.color = sf::Color::Blue;
    style_curve.thickness = 2.0;
    style_curve.legend = "y = x" + SQUARED;
    
    style_text1.color = sf::Color::Red;
    style_text1.thickness = 3.0;  // Larger text
    
    style_text2.color = sf::Color(255, 165, 0);
    style_text2.thickness = 2.0;  // Medium text
    
    style_text3.color = sf::Color::Magenta;
    style_text3.thickness = 1.5;  // Smaller text
    
    style_arc1.color = sf::Color::Black;
    style_arc1.thickness = 2.5;
    style_arc1.legend = "180° arc";
    
    style_arc2.color = sf::Color(255, 165, 0); // Orange
    style_arc2.thickness = 2.0;
    style_arc2.line_style = "dashed";
    style_arc2.legend = "90° arc";
    
    plt.set_title(fig2, "Parabola with text and arcs");
    plt.set_xlabel(fig2, "X");
    plt.set_ylabel(fig2, "Y");
    plt.set_axis_limits(fig2, -5, 5, -1, 25);
    plt.grid(fig2, true, false);
    
    // Plot the parabola
    plt.plot(fig2, x, y, style_curve);
    
    // Add text annotations at different positions
    plt.text(fig2, 0, 0, "Origin (0,0)", style_text1);
    plt.text(fig2, -3, 9, "y = x²", style_text2);
    plt.text(fig2, 4, 16, "Increasing slope here", style_text3);
    
    // Add arcs to highlight regions of the parabola
    plt.arc(fig2, 0, 0, 3, 0, M_PI, style_arc1);        // 180° arc centered at origin
    plt.arc(fig2, 2, 4, 2, M_PI/4, M_PI/4*3, style_arc2); // 90° arc elsewhere
    
    // Example 3: Sine wave with arrows and tangent lines
    auto& fig3 = plt.subplot(1, 0);
    
    // Generate data for a sine wave
    std::vector<double> x_sin(200), y_sin(200);
    for (int i = 0; i < 200; ++i) {
        x_sin[i] = i * 0.05;
        y_sin[i] = sin(x_sin[i]);
    }
    
    // Create styles for sine curve, arrows and lines
    PlotGen::Style style_sin, style_arrow1, style_arrow2, style_arrow3;
    PlotGen::Style style_tangent1, style_tangent2;
    
    style_sin.color = sf::Color::Blue;
    style_sin.thickness = 2.0;
    style_sin.legend = "sin(x)";
    
    style_arrow1.color = sf::Color::Red;
    style_arrow1.thickness = 1.5;
    
    style_arrow2.color = sf::Color::Green;
    style_arrow2.thickness = 2.0;
    
    style_arrow3.color = sf::Color(255, 165, 0); // Orange
    style_arrow3.thickness = 3.0;
    
    style_tangent1.color = sf::Color::Cyan;
    style_tangent1.thickness = 1.5;
    style_tangent1.line_style = "dashed";
    style_tangent1.legend = "Tangent line at x=π";
    
    style_tangent2.color = sf::Color::Yellow;
    style_tangent2.thickness = 1.5;
    style_tangent2.line_style = "dashed";
    style_tangent2.legend = "Tangent line at x=2π";
    
    plt.set_title(fig3, "Sine wave with arrows and tangents");
    plt.set_xlabel(fig3, "X");
    plt.set_ylabel(fig3, "Y");
    plt.set_axis_limits(fig3, 0, 10, -1.5, 1.5);
    plt.grid(fig3, true, false);
    
    // Plot the sine wave
    plt.plot(fig3, x_sin, y_sin, style_sin);
    
    // Draw arrows pointing to interesting features
    plt.arrow(fig3, 2.2, -1, 1.57, 0, style_arrow1, 20.0);    // Arrow to first minimum
    plt.arrow(fig3, 6.5, 1, 4.71, 0, style_arrow2, 30.0);    // Arrow to second maximum
    plt.arrow(fig3, 8.5, -1, 7.85, 0, style_arrow3, 50.0);   // Arrow to third minimum
    
    // Add text to explain the arrows
    plt.text(fig3, 2.2, -1.2, "First minimum", PlotGen::Style(sf::Color::Red));
    plt.text(fig3, 6.5, 1.2, "Second maximum", PlotGen::Style(sf::Color::Green));
    plt.text(fig3, 8.5, -1.2, "Third minimum", PlotGen::Style(sf::Color(255, 165, 0)));
    
    // Add tangent lines at specific points
    double x_pi = M_PI;
    double y_pi = sin(x_pi);        // y = 0 at x = π
    double slope_pi = cos(x_pi);    // derivative = cos(x)
    
    double x_2pi = 2 * M_PI;
    double y_2pi = sin(x_2pi);      // y = 0 at x = 2π
    double slope_2pi = cos(x_2pi);  // derivative = cos(x)
    
    // Draw tangent lines (y = y0 + slope*(x-x0))
    plt.line(fig3, x_pi - 1, y_pi - slope_pi, x_pi + 1, y_pi + slope_pi, style_tangent1);
    plt.line(fig3, x_2pi - 1, y_2pi - slope_2pi, x_2pi + 1, y_2pi + slope_2pi, style_tangent2);
    
    // Example 4: Vector field with arrows, lines and arcs
    auto& fig4 = plt.subplot(1, 1);
    
    plt.set_title(fig4, "Vector field with geometric elements");
    plt.set_xlabel(fig4, "X");
    plt.set_ylabel(fig4, "Y");
    plt.set_axis_limits(fig4, -5, 5, -5, 5);
    plt.grid(fig4, true, true);
    plt.set_equal_axes(fig4, true);
    
    // Draw a vector field (circular pattern)
    for (int i = -4; i <= 4; i += 2) {
        for (int j = -4; j <= 4; j += 2) {
            double x = i;
            double y = j;
            
            // Calculate vector direction (perpendicular to radius)
            double r = sqrt(x*x + y*y);
            
            if (r < 0.1) continue; // Skip center point
            
            double scale = 0.8; // Scale factor for arrow length
            double dx = -y/r * scale;
            double dy = x/r * scale;
            
            // Create arrow style with color based on distance from origin
            PlotGen::Style arrow_style;
            arrow_style.thickness = 1.5;
            
            // Color gradient: blue at center to red at edges
            double dist_norm = r / 5.0; // Normalize to [0,1]
            arrow_style.color = sf::Color(
                static_cast<sf::Uint8>(255 * dist_norm),  // Red
                0,                                         // Green
                static_cast<sf::Uint8>(255 * (1-dist_norm)) // Blue
            );
            
            // Draw arrow from (x,y) with calculated direction
            plt.arrow(fig4, x, y, x + dx, y + dy, arrow_style, 10.0);
        }
    }
    
    // Add concentric circles
    PlotGen::Style circle_style1, circle_style2, circle_style3;
    circle_style1.color = sf::Color::Black;
    circle_style1.thickness = 2.0;
    circle_style1.legend = "r = 1 circle";
    
    circle_style2.color = sf::Color(255, 165, 0); // Orange
    circle_style2.thickness = 1.5;
    circle_style2.line_style = "dashed";
    circle_style2.legend = "r = 3 circle";
    
    // Draw concentric circles
    plt.circle(fig4, 0, 0, 0.2, circle_style1);
    plt.circle(fig4, 0, 0, 1, circle_style1);
    plt.circle(fig4, 0, 0, 3, circle_style2);
    
    // Add some connecting lines
    PlotGen::Style line_style;
    line_style.color = sf::Color::Cyan;
    line_style.thickness = 1.0;
    line_style.line_style = "dashed";
    
    // Connect points with lines
    plt.line(fig4, -4, -4, 4, 4, line_style);
    plt.line(fig4, -4, 4, 4, -4, line_style);
    
    // Add arcs
    PlotGen::Style arc_style;
    arc_style.color = sf::Color::Magenta;
    arc_style.thickness = 2.0;
    arc_style.legend = "90° arc";
    
    // Draw an arc in each quadrant
    plt.arc(fig4, 0, 0, 2, 0, M_PI/2, arc_style);         // First quadrant
    plt.arc(fig4, 0, 0, 2, M_PI/2, M_PI, arc_style);      // Second quadrant
    plt.arc(fig4, 0, 0, 2, M_PI, 3*M_PI/2, arc_style);    // Third quadrant
    plt.arc(fig4, 0, 0, 2, 3*M_PI/2, 2*M_PI, arc_style);  // Fourth quadrant
    
    // Add text label for the vector field
    plt.text(fig4, 0, 4, "Circular vector field", PlotGen::Style(sf::Color::White, 2.5));
    
    plt.save("example7_circles_text_arrows.png");
    plt.show();
}

// Example 8: Bezier and Spline Curves
void example_bezier_spline() {
    PlotGen plt(1200, 900, 2, 2);
    
    // Example 1: Cubic Bezier curve
    auto& fig1 = plt.subplot(0, 0);
    
    // Define control points for a cubic Bezier curve
    double x0 = -4, y0 = 0;   // Starting point
    double x1 = -1, y1 = 4;   // First control point
    double x2 = 1, y2 = -4;   // Second control point
    double x3 = 4, y3 = 0;    // End point
    
    PlotGen::Style style_bezier, style_cp, style_lines;
    
    style_bezier.color = sf::Color::Blue;
    style_bezier.thickness = 3.0;
    style_bezier.legend = "Cubic Bezier curve";
    
    style_cp.color = sf::Color::Red;
    style_cp.symbol_type = "circle";
    style_cp.symbol_size = 10.0;
    style_cp.line_style = "none";
    style_cp.legend = "Control points";
    
    style_lines.color = sf::Color(100, 100, 100); // Gray
    style_lines.thickness = 1.0;
    style_lines.line_style = "dashed";
    style_lines.legend = "Control polygon";
    
    plt.set_title(fig1, "Cubic Bezier Curve");
    plt.set_xlabel(fig1, "X");
    plt.set_ylabel(fig1, "Y");
    plt.grid(fig1, true, false);
    
    // Draw the Bezier curve
    plt.bezier(fig1, x0, y0, x1, y1, x2, y2, x3, y3, style_bezier);
    
    // Draw the control points
    std::vector<double> cp_x = {x0, x1, x2, x3};
    std::vector<double> cp_y = {y0, y1, y2, y3};
    plt.plot(fig1, cp_x, cp_y, style_cp);
    
    // Draw the control polygon (lines connecting control points)
    plt.plot(fig1, cp_x, cp_y, style_lines);
    
    // Example 2: Multiple Bezier curves with different control points
    auto& fig2 = plt.subplot(0, 1);
    
    // Define several sets of control points for Bezier curves
    std::vector<std::vector<double>> bezier_points = {
        {-4, -3, -1, 0},  // First curve: control points x-coordinates
        {-3, 0, 0, -3},   // First curve: control points y-coordinates
        {0, 1, 3, 4},     // Second curve: control points x-coordinates
        {-3, 0, 0, 3},    // Second curve: control points y-coordinates
        {-2, 0, 0, 2},    // Third curve: control points x-coordinates
        {0, 3, -3, 0}     // Third curve: control points y-coordinates
    };
    
    std::vector<sf::Color> colors = {
        sf::Color::Red,
        sf::Color::Blue,
        sf::Color::Green
    };
    
    plt.set_title(fig2, "Multiple Bezier Curves");
    plt.set_xlabel(fig2, "X");
    plt.set_ylabel(fig2, "Y");
    plt.set_axis_limits(fig2, -5, 5, -5, 5);
    plt.grid(fig2, true, false);
    
    // Draw each Bezier curve and its control points
    for (size_t i = 0; i < 3; ++i) {
        PlotGen::Style curve_style, cp_style;
        
        curve_style.color = colors[i];
        curve_style.thickness = 3.0;
        curve_style.legend = "Bezier curve " + std::to_string(i+1);
        
        cp_style.color = colors[i];
        cp_style.symbol_type = "circle";
        cp_style.symbol_size = 6.0;
        cp_style.line_style = "dashed";
        cp_style.legend = "Control points " + std::to_string(i+1);
        
        // Extract control points for this curve
        std::vector<double> x_points = {
            bezier_points[i*2][0], 
            bezier_points[i*2][1], 
            bezier_points[i*2][2], 
            bezier_points[i*2][3]
        };
        std::vector<double> y_points = {
            bezier_points[i*2+1][0], 
            bezier_points[i*2+1][1], 
            bezier_points[i*2+1][2], 
            bezier_points[i*2+1][3]
        };
        
        // Draw the Bezier curve
        plt.bezier(fig2, x_points, y_points, curve_style);
        
        // Draw the control points and connecting lines
        plt.plot(fig2, x_points, y_points, cp_style);
    }
    
    // Example 3: Natural Cubic Spline
    auto& fig3 = plt.subplot(1, 0);
    
    // Points for the natural cubic spline
    std::vector<double> spline_x = {-4, -2, 0, 2, 4};
    std::vector<double> spline_y = {-2, 3, 0, 1, -1};
    
    PlotGen::Style style_spline, style_points;
    
    style_spline.color = sf::Color::Red;
    style_spline.thickness = 3.0;
    style_spline.legend = "Natural cubic spline";
    
    style_points.color = sf::Color::Blue;
    style_points.symbol_type = "circle";
    style_points.symbol_size = 8.0;
    style_points.line_style = "none";
    style_points.legend = "Data points";
    
    plt.set_title(fig3, "Natural Cubic Spline");
    plt.set_xlabel(fig3, "X");
    plt.set_ylabel(fig3, "Y");
    plt.grid(fig3, true, false);
    
    // Draw the natural cubic spline
    plt.spline(fig3, spline_x, spline_y, style_spline);
    
    // Draw the spline points
    plt.plot(fig3, spline_x, spline_y, style_points);
    
    // Example 4: Cardinal Spline with tension parameter
    auto& fig4 = plt.subplot(1, 1);
    
    // Points for the cardinal spline
    std::vector<double> cardinal_x = {-4, -3, -1, 1, 3, 4};
    std::vector<double> cardinal_y = {0, -2, 1, -1, 2, 0};
    
    // Different tensions for cardinal splines
    std::vector<double> tensions = {0.0, 0.5, 1.0};
    std::vector<sf::Color> tension_colors = {
        sf::Color::Green,
        sf::Color::Blue,
        sf::Color::Red
    };
    
    plt.set_title(fig4, "Cardinal Splines with Different Tensions");
    plt.set_xlabel(fig4, "X");
    plt.set_ylabel(fig4, "Y");
    plt.grid(fig4, true, false);
    
    // Draw cardinal splines with different tension values
    for (size_t i = 0; i < tensions.size(); ++i) {
        PlotGen::Style spline_style;
        
        spline_style.color = tension_colors[i];
        spline_style.thickness = 2.5;
        spline_style.legend = "Tension = " + std::to_string(tensions[i]);
        
        plt.cardinal_spline(fig4, cardinal_x, cardinal_y, tensions[i], spline_style);
    }
    
    // Draw the data points
    PlotGen::Style points_style;
    points_style.color = sf::Color::Cyan;
    points_style.symbol_type = "circle";
    points_style.symbol_size = 8.0;
    points_style.line_style = "none";
    points_style.legend = "Data points";
    
    plt.plot(fig4, cardinal_x, cardinal_y, points_style);
    
    plt.save("example8_bezier_spline.png");
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
    std::cout << "7. Circles, Text and Arrows" << std::endl;
    std::cout << "8. Bezier and Spline Curves" << std::endl;
    std::cout << "Enter your choice (1-8): ";
    
    int choice;
    std::cin >> choice;
    
    switch (choice) {
        case 1: example_basic_plots(); break;
        case 2: example_histograms(); break;
        case 3: example_polar_plots(); break;
        case 4: example_multiple_plots(); break;
        case 5: example_advanced_histograms(); break;
        case 6: example_symbol_plots(); break;
        case 7: example_circles_text_arrows(); break;
        case 8: example_bezier_spline(); break;
        default: 
            std::cout << "Invalid choice." << std::endl;
            return 1;
    }
    
    return 0;
}