#include "plotgen.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <cstdlib> // Pour system()
#ifdef HAVE_GTK_WEBKIT
#include <webkit2/webkit2.h>
#include <gtk/gtk.h>
#endif
#include <sstream>
#include <ctime>

// Style struct constructor implementation
PlotGen::Style::Style(
    sf::Color color_,
    double thickness_,
    const std::string &line_style_,
    const std::string &legend_,
    const std::string &symbol_type_,
    double symbol_size_) : color(color_), thickness(thickness_), line_style(line_style_),
                           legend(legend_), symbol_type(symbol_type_), symbol_size(symbol_size_)
{
}

// Constructor
PlotGen::PlotGen(unsigned int width, unsigned int height, unsigned int rows, unsigned int cols)
    : width(width), height(height), rows(rows), cols(cols) // Ne pas initialiser la fenêtre ici
{
    // Créer uniquement la texture pour le rendu, mais pas la fenêtre visible
    texture.create(width, height);
    texture.setSmooth(true);

    // Search for the font in several possible locations
    if (font.loadFromFile("fonts/arial.ttf"))
    {
        std::cout << "Font loaded from current directory" << std::endl;
    }
    else if (font.loadFromFile("build/arial.ttf"))
    {
        std::cout << "Font loaded from build directory" << std::endl;
    }
    else if (font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"))
    {
        std::cout << "LiberationSans font loaded" << std::endl;
    }
    else if (font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
    {
        std::cout << "DejaVuSans font loaded" << std::endl;
    }
    else
    {
        std::cerr << "WARNING: Unable to load a font compatible with Unicode characters" << std::endl;
        throw std::runtime_error("Unable to load a font supporting Unicode");
    }

    figures.resize(rows * cols);
    for (auto &fig : figures)
    {
        fig.curve_types.clear();
        fig.curves.clear();
    }
}

// Add a figure at a position (row, col)
PlotGen::Figure &PlotGen::subplot(unsigned int row, unsigned int col)
{
    if (row >= rows || col >= cols)
    {
        throw std::out_of_range("Subplot index out of range");
    }
    return figures[row * cols + col];
}

// Figure configuration
void PlotGen::set_title(Figure &fig, const std::string &title) { fig.title = title; }
void PlotGen::set_xlabel(Figure &fig, const std::string &label) { fig.xlabel = label; }
void PlotGen::set_ylabel(Figure &fig, const std::string &label) { fig.ylabel = label; }
void PlotGen::set_axis_limits(Figure &fig, double xmin, double xmax, double ymin, double ymax)
{
    fig.xmin = xmin;
    fig.xmax = xmax;
    fig.ymin = ymin;
    fig.ymax = ymax;
}

// New function specific for polar graphs ensuring axes of the same dimension
void PlotGen::set_polar_axis_limits(Figure &fig, double max_radius)
{
    // For a polar graph, X and Y axes must have the same scale
    // and be centered on (0,0)
    fig.is_polar = true; // Mark as a polar graph
    fig.xmin = -max_radius;
    fig.xmax = max_radius;
    fig.ymin = -max_radius;
    fig.ymax = max_radius;
}

void PlotGen::show_legend(Figure &fig, bool show) { fig.show_leg = show; }

// Nouvelle méthode pour définir la position de la légende
void PlotGen::set_legend_position(Figure &fig, const std::string &position)
{
    // Vérifier que la position est valide
    if (position == "top-right" || position == "top-left" ||
        position == "bottom-right" || position == "bottom-left" ||
        position == "outside-right")
    {
        fig.legend_position = position;
    }
    else
    {
        std::cerr << "WARNING: Invalid legend position. Using default 'top-right'." << std::endl;
        fig.legend_position = "top-right";
    }
}

// Methods to enable/disable grids
void PlotGen::grid(Figure &fig, bool major, bool minor)
{
    fig.show_major_grid = major;
    fig.show_minor_grid = minor;
}

void PlotGen::set_grid_color(Figure &fig, sf::Color major_color, sf::Color minor_color)
{
    fig.major_grid_color = major_color;
    fig.minor_grid_color = minor_color;
}

void PlotGen::set_equal_axes(Figure &fig, bool equal) { fig.equal_axes = equal; }

// 2D curve plotting
void PlotGen::plot(Figure &fig, const std::vector<double> &x, const std::vector<double> &y, const Style &style)
{
    if (x.size() != y.size() || x.empty())
    {
        throw std::invalid_argument("x and y vectors must have the same size and not be empty");
    }

    // Ajouter un auto-ajustement des limites d'axes si elles n'ont pas été définies
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);

    if (using_default_limits)
    {
        // Trouver les min/max pour x et y
        double x_min = *std::min_element(x.begin(), x.end());
        double x_max = *std::max_element(x.begin(), x.end());
        double y_min = *std::min_element(y.begin(), y.end());
        double y_max = *std::max_element(y.begin(), y.end());

        // Ajouter une marge de 5% pour une meilleure visualisation
        double x_margin = (x_max - x_min) * 0.05;
        double y_margin = (y_max - y_min) * 0.05;

        // Cas particulier pour éviter des problèmes si toutes les valeurs sont égales
        if (std::abs(x_max - x_min) < 1e-10)
        {
            x_margin = 1.0;
        }
        if (std::abs(y_max - y_min) < 1e-10)
        {
            y_margin = 1.0;
        }

        // Définir les nouvelles limites
        fig.xmin = x_min - x_margin;
        fig.xmax = x_max + x_margin;
        fig.ymin = y_min - y_margin;
        fig.ymax = y_max + y_margin;
    }

    // Utiliser directement le style sans ajouter de description textuelle
    fig.curves.push_back({x, y, style});
    fig.curve_types.push_back("2D");
}

// Circle with center (x0, y0) and radius r
void PlotGen::circle(Figure &fig, double x0, double y0, double r, const Style &style)
{
    // Validation du rayon
    if (r <= 0)
    {
        throw std::invalid_argument("Circle radius must be positive");
    }

    // Générer les points du cercle en utilisant les équations paramétriques
    const int num_points = 100; // Nombre de points pour dessiner le cercle
    std::vector<double> x(num_points), y(num_points);

    for (int i = 0; i < num_points; ++i)
    {
        double angle = 2.0 * M_PI * i / (num_points - 1);
        x[i] = x0 + r * std::cos(angle);
        y[i] = y0 + r * std::sin(angle);
    }

    // Activer l'option equal_axes pour garantir que le cercle est bien rond
    bool was_equal = fig.equal_axes;
    fig.equal_axes = true;

    // Vérifier si les limites d'axes existantes sont suffisantes
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);

    if (using_default_limits)
    {
        // Définir de nouvelles limites d'axes pour bien voir le cercle avec une marge
        double margin = r * 0.2; // 20% de marge autour du cercle
        fig.xmin = x0 - r - margin;
        fig.xmax = x0 + r + margin;
        fig.ymin = y0 - r - margin;
        fig.ymax = y0 + r + margin;
    }
    else
    {
        // Vérifier si le cercle est visible dans les limites actuelles
        double min_x = x0 - r;
        double max_x = x0 + r;
        double min_y = y0 - r;
        double max_y = y0 + r;

        // Étendre les limites si nécessaire
        if (min_x < fig.xmin)
            fig.xmin = min_x - r * 0.1;
        if (max_x > fig.xmax)
            fig.xmax = max_x + r * 0.1;
        if (min_y < fig.ymin)
            fig.ymin = min_y - r * 0.1;
        if (max_y > fig.ymax)
            fig.ymax = max_y + r * 0.1;
    }

    // Tracer le cercle comme une courbe normale
    plot(fig, x, y, style);

    // Restaurer l'état précédent de equal_axes si nécessaire
    fig.equal_axes = was_equal;
}

// Arc centered at (x0, y0) from angle1 to angle2 (in radians) with radius r
void PlotGen::arc(Figure &fig, double x0, double y0, double r, double angle1, double angle2, const Style &style, int num_points)
{
    // Validate inputs
    if (r <= 0)
    {
        throw std::invalid_argument("Arc radius must be positive");
    }

    if (num_points < 2)
    {
        throw std::invalid_argument("Number of points must be at least 2");
    }

    // Ensure angle1 is less than angle2
    if (angle1 > angle2)
    {
        std::swap(angle1, angle2);
    }

    // Generate points for the arc using parametric equations
    std::vector<double> x(num_points), y(num_points);

    for (int i = 0; i < num_points; ++i)
    {
        double angle = angle1 + (angle2 - angle1) * i / (num_points - 1);
        x[i] = x0 + r * std::cos(angle);
        y[i] = y0 + r * std::sin(angle);
    }

    // Check if we need to adjust axis limits
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);

    if (using_default_limits)
    {
        // Calculate arc bounds
        double min_x = x0 - r;
        double max_x = x0 + r;
        double min_y = y0 - r;
        double max_y = y0 + r;

        // Add margin (20% of radius)
        double margin = r * 0.2;

        fig.xmin = min_x - margin;
        fig.xmax = max_x + margin;
        fig.ymin = min_y - margin;
        fig.ymax = max_y + margin;

        // Use equal_axes to maintain circular appearance
        fig.equal_axes = true;
    }
    else
    {
        // Check if the arc fits within current limits
        double min_x = x0 + r * std::min(std::cos(angle1), std::cos(angle2));
        double max_x = x0 + r * std::max(std::cos(angle1), std::cos(angle2));
        double min_y = y0 + r * std::min(std::sin(angle1), std::sin(angle2));
        double max_y = y0 + r * std::max(std::sin(angle1), std::sin(angle2));

        // Also check extreme points (0, π/2, π, 3π/2) if they fall within the angle range
        if (angle1 <= 0 && angle2 >= 0)
            max_x = std::max(max_x, x0 + r);
        if (angle1 <= M_PI / 2 && angle2 >= M_PI / 2)
            max_y = std::max(max_y, y0 + r);
        if (angle1 <= M_PI && angle2 >= M_PI)
            min_x = std::min(min_x, x0 - r);
        if (angle1 <= 3 * M_PI / 2 && angle2 >= 3 * M_PI / 2)
            min_y = std::min(min_y, y0 - r);

        // Extend limits if needed
        if (min_x < fig.xmin)
            fig.xmin = min_x - r * 0.1;
        if (max_x > fig.xmax)
            fig.xmax = max_x + r * 0.1;
        if (min_y < fig.ymin)
            fig.ymin = min_y - r * 0.1;
        if (max_y > fig.ymax)
            fig.ymax = max_y + r * 0.1;
    }

    // Store the arc as a 2D curve
    fig.curves.push_back({x, y, style});
    fig.curve_types.push_back("2D");
}

// Text at a specific position (x, y) in data coordinates
void PlotGen::text(Figure &fig, double x, double y, const std::string &text_content, const Style &style)
{
    if (text_content.empty())
    {
        return; // Nothing to render
    }

    // Create a new data structure for storing the text position and content
    // We'll use the plot method's data structures but modify the type
    std::vector<double> x_pos = {x};
    std::vector<double> y_pos = {y};

    // Store the text content in the curve structure, but mark it with a special type
    // We'll use the "TEXT" type to distinguish it from other curve types
    Figure::Curve text_curve = {x_pos, y_pos, style};
    text_curve.text_content = text_content; // We'll need to add this field to the Curve struct

    fig.curves.push_back(text_curve);
    fig.curve_types.push_back("TEXT");

    // Check if we need to adjust the axis limits to include the text position
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);
    if (using_default_limits)
    {
        // If using default limits, adjust to include the text position with a margin
        double margin_x = std::abs(x) * 0.2 + 1.0; // 20% margin or at least 1.0 unit
        double margin_y = std::abs(y) * 0.2 + 1.0;

        fig.xmin = std::min(fig.xmin, x - margin_x);
        fig.xmax = std::max(fig.xmax, x + margin_x);
        fig.ymin = std::min(fig.ymin, y - margin_y);
        fig.ymax = std::max(fig.ymax, y + margin_y);
    }
    else
    {
        // Check if the text position is outside the current limits
        if (x < fig.xmin || x > fig.xmax || y < fig.ymin || y > fig.ymax)
        {
            // Expand limits to include the text position with a small margin
            double margin_x = (fig.xmax - fig.xmin) * 0.05;
            double margin_y = (fig.ymax - fig.ymin) * 0.05;

            if (x < fig.xmin)
                fig.xmin = x - margin_x;
            if (x > fig.xmax)
                fig.xmax = x + margin_x;
            if (y < fig.ymin)
                fig.ymin = y - margin_y;
            if (y > fig.ymax)
                fig.ymax = y + margin_y;
        }
    }
}

// Arrow from (x1,y1) to (x2,y2)
void PlotGen::arrow(Figure &fig, double x1, double y1, double x2, double y2, const Style &style, double head_size)
{
    // Validate inputs
    if (head_size <= 0)
    {
        throw std::invalid_argument("Arrow head size must be positive");
    }

    // First, draw the line from start to end point
    std::vector<double> x = {x1, x2};
    std::vector<double> y = {y1, y2};

    // We'll handle the line drawing here to avoid auto-adjusting axis twice
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);

    if (using_default_limits)
    {
        // Auto-adjust axis limits to include the arrow with some margin
        double x_min = std::min(x1, x2);
        double x_max = std::max(x1, x2);
        double y_min = std::min(y1, y2);
        double y_max = std::max(y1, y2);

        // Add margin (10% of the data range or at least 1.0 unit)
        double x_margin = std::max((x_max - x_min) * 0.1, 1.0);
        double y_margin = std::max((y_max - y_min) * 0.1, 1.0);

        fig.xmin = x_min - x_margin;
        fig.xmax = x_max + x_margin;
        fig.ymin = y_min - y_margin;
        fig.ymax = y_max + y_margin;
    }

    // Store the arrow shaft as a regular 2D curve
    fig.curves.push_back({x, y, style});
    fig.curve_types.push_back("2D");

    // Store points for the arrowhead
    // We'll calculate these in screen coordinates during rendering
    // Just store the data points and head_size for now
    Figure::Curve arrow_head_curve;
    arrow_head_curve.x = {x1, x2}; // Store start and end points
    arrow_head_curve.y = {y1, y2};
    arrow_head_curve.style = style;
    arrow_head_curve.head_size = head_size; // Store head size as a property

    // Add the arrowhead to the figure
    fig.curves.push_back(arrow_head_curve);
    fig.curve_types.push_back("ARROW_HEAD");
}

// Line from (x1,y1) to (x2,y2)
void PlotGen::line(Figure &fig, double x1, double y1, double x2, double y2, const Style &style)
{
    // Create vector of points for the line
    std::vector<double> x = {x1, x2};
    std::vector<double> y = {y1, y2};

    // Check if we need to adjust axis limits
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);

    if (using_default_limits)
    {
        // Auto-adjust axis limits to include the line with some margin
        double x_min = std::min(x1, x2);
        double x_max = std::max(x1, x2);
        double y_min = std::min(y1, y2);
        double y_max = std::max(y1, y2);

        // Add margin (10% of the data range or at least 1.0 unit)
        double x_margin = std::max((x_max - x_min) * 0.1, 1.0);
        double y_margin = std::max((y_max - y_min) * 0.1, 1.0);

        // Handle the case where x_min == x_max or y_min == y_max
        if (std::abs(x_max - x_min) < 1e-10)
        {
            x_margin = 1.0;
        }
        if (std::abs(y_max - y_min) < 1e-10)
        {
            y_margin = 1.0;
        }

        fig.xmin = x_min - x_margin;
        fig.xmax = x_max + x_margin;
        fig.ymin = y_min - y_margin;
        fig.ymax = y_max + y_margin;
    }

    // Store the line as a regular 2D curve
    fig.curves.push_back({x, y, style});
    fig.curve_types.push_back("2D");
}

// Histogram
void PlotGen::hist(Figure &fig, const std::vector<double> &data, int bins, const Style &style, double bar_width_ratio)
{
    if (data.empty())
    {
        throw std::invalid_argument("data vector must not be empty");
    }

    std::vector<double> hist_x, hist_y;
    double min_val = *std::min_element(data.begin(), data.end());
    double max_val = *std::max_element(data.begin(), data.end());

    // Protection contre le cas où min_val == max_val
    if (std::abs(max_val - min_val) < 1e-10)
    {
        // Ajouter une petite variation pour éviter une division par zéro
        min_val -= 0.5;
        max_val += 0.5;
    }

    double bin_width = (max_val - min_val) / bins;

    // Initialiser les comptages à zéro
    std::vector<int> counts(bins, 0);

    // Calculer les histogrammes
    for (double val : data)
    {
        int bin = std::min(static_cast<int>((val - min_val) / bin_width), bins - 1);
        // Protection contre les indices négatifs
        bin = std::max(0, bin);
        counts[bin]++;
    }

    // Construire les données d'histogramme
    for (int i = 0; i < bins; ++i)
    {
        hist_x.push_back(min_val + i * bin_width);
        hist_y.push_back(static_cast<double>(counts[i]));
    }

    // Automatically set axis limits if they haven't been specified
    // Check if default limits are being used
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);

    if (using_default_limits)
    {
        // Set X limits to include all bars with a 5% margin
        double x_margin = (max_val - min_val) * 0.05f;
        fig.xmin = min_val - x_margin;
        fig.xmax = max_val + bin_width + x_margin; // Add bin_width to see the last bar completely

        // Set Y limits to include all bars with a 10% margin
        double max_count = *std::max_element(hist_y.begin(), hist_y.end());
        double y_margin = max_count * 0.1f;
        fig.ymin = 0; // Histograms generally start at 0
        fig.ymax = max_count + y_margin;
    }

    // Create a curve with the specified bar width ratio
    Figure::Curve curve = {hist_x, hist_y, style};
    curve.bar_width_ratio = bar_width_ratio; // Apply bar width ratio

    fig.curves.push_back(curve);
    fig.curve_types.push_back("HIST");
}

// Polar plot
void PlotGen::polar_plot(Figure &fig, const std::vector<double> &theta, const std::vector<double> &r, const Style &style)
{
    if (theta.size() != r.size() || theta.empty())
    {
        throw std::invalid_argument("theta and r vectors must have the same size and not be empty");
    }

    // Mark the figure as being in polar coordinates
    fig.is_polar = true;

    // Find the maximum radius to adapt the graph limits
    double max_r = *std::max_element(r.begin(), r.end());

    // Check if we're using default limits
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);

    // If the limits haven't been explicitly set or are default, calculate them automatically
    if (using_default_limits)
    {
        // Add a 10% margin for better visualization
        set_polar_axis_limits(fig, max_r * 1.1f);
    }
    else
    {
        // Check if current limits are sufficient to contain all data
        // and if they respect polar graph constraints (axes of same dimension)
        double current_range_x = (fig.xmax - fig.xmin) / 2;
        double current_range_y = (fig.ymax - fig.ymin) / 2;
        double needed_range = max_r * 1.1f;

        // Ensure axes have the same scale
        double max_range = std::max(current_range_x, current_range_y);

        if (needed_range > max_range || std::abs(current_range_x - current_range_y) > 1e-6)
        {
            // Use set_polar_axis_limits to ensure axes of same dimension
            double max_limit = std::max(needed_range, max_range);
            set_polar_axis_limits(fig, max_limit);
        }
    }

    // Convert to Cartesian coordinates for display
    std::vector<double> x, y;
    for (size_t i = 0; i < theta.size(); ++i)
    {
        x.push_back(r[i] * std::cos(theta[i]));
        y.push_back(r[i] * std::sin(theta[i]));
    }

    fig.curves.push_back({x, y, style});
    fig.curve_types.push_back("POLAR");
}

// Display and render
void PlotGen::show()
{

#ifdef HAVE_GTK_WEBKIT
    show_with_viewer();
#else
    showSFML();
#endif
}

// Méthode privée pour l'affichage SFML original
void PlotGen::showSFML()
{
    // Effectuer le rendu
    render();

    // Créer la fenêtre seulement lorsque show() est explicitement appelé
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    window.create(sf::VideoMode(width, height), "PlotGen", sf::Style::Default, settings);

    // Configurer le sprite
    sprite.setTexture(texture.getTexture());

    // Main loop
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
            }
        }

        // Render and display
        window.clear(sf::Color::White);
        window.draw(sprite);
        window.display();
    }
}

std::string PlotGen::get_svg_in_html(const std::string &svg_filename)
{
    // Lire le contenu du fichier SVG
    std::ifstream svg_file(svg_filename);
    if (!svg_file.is_open())
    {
        std::cerr << "Erreur: Impossible d'ouvrir le fichier SVG" << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << svg_file.rdbuf();
    svg_file.close();

    // Créer le contenu HTML
    std::string html_content = "<!DOCTYPE html>\n"
                               "<html>\n"
                               "<head>\n"
                               "    <meta charset=\"UTF-8\">\n"
                               "    <title>PlotGenC++ - Visualisation SVG</title>\n"
                               "    <style>\n"
                               "        body { margin: 0; padding: 0; overflow: hidden; background-color: #f0f0f0; }\n"
                               "        #svg-container { width: 100vw; height: 100vh; display: flex; justify-content: center; align-items: center; }\n"
                               "        #controls { position: fixed; bottom: 10px; left: 10px; background: rgba(255,255,255,0.8); padding: 10px; border-radius: 5px; }\n"
                               "        svg { max-width: 95%; max-height: 95%; background-color: white; box-shadow: 0 0 10px rgba(0,0,0,0.2); }\n"
                               "    </style>\n"
                               "</head>\n"
                               "<body>\n"
                               "    <div id=\"svg-container\">\n" +
                               buffer.str() + "\n"
                                              "    </div>\n"
                                              "    <div id=\"controls\">\n"
                                              "        <button onclick=\"window.close()\">Fermer</button>\n"
                                              "        <button onclick=\"saveSvg()\">Télécharger SVG</button>\n"
                                              "    </div>\n"
                                              "    <script>\n"
                                              "        function saveSvg() {\n"
                                              "            const link = document.createElement('a');\n"
                                              "            link.href = '" +
                               svg_filename + "';\n"
                                              "            link.download = '" +
                               svg_filename + "';\n"
                                              "            document.body.appendChild(link);\n"
                                              "            link.click();\n"
                                              "            document.body.removeChild(link);\n"
                                              "        }\n"
                                              "        document.addEventListener('keydown', function(e) {\n"
                                              "            if (e.key === 'Escape') window.close();\n"
                                              "        });\n"
                                              "    </script>\n"
                                              "</body>\n"
                                              "</html>";

    return html_content;
}

// Save
void PlotGen::save(const std::string &filename)
{
    render(); // Ensure the rendering is up to date

    // Vérifier si c'est un fichier SVG et utiliser l'export vectoriel
    if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".svg") == 0)
    {
        save_svg(filename);
        return;
    }

    sf::Image screenshot = texture.getTexture().copyToImage();

    if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".png") == 0)
    {
        if (!screenshot.saveToFile(filename))
        {
            throw std::runtime_error("Unable to save image in PNG format");
        }
    }
    else if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".jpg") == 0)
    {
        // Conversion for stb_image_write with high quality
        std::vector<unsigned char> pixels(width * height * 3); // RGB
        for (unsigned int y = 0; y < height; ++y)
        {
            for (unsigned int x = 0; x < width; ++x)
            {
                sf::Color c = screenshot.getPixel(x, y);
                size_t index = (y * width + x) * 3;
                pixels[index] = c.r;
                pixels[index + 1] = c.g;
                pixels[index + 2] = c.b;
            }
        }

        // Write JPG image with 95% quality (better quality)
        if (!stbi_write_jpg(filename.c_str(), width, height, 3, pixels.data(), 95))
        {
            throw std::runtime_error("Unable to save image in JPG format");
        }
    }
    else
    {
        throw std::invalid_argument("File format not supported. Use .png, .jpg or .svg");
    }

    std::cout << "Image saved to: " << filename << " (Dimensions: " << width << "x" << height << ")" << std::endl;
}

// Implémentation de la méthode d'export SVG
void PlotGen::save_svg(const std::string &filename)
{
    // Créer un fichier SVG
    std::ofstream svg_file(filename);
    if (!svg_file.is_open())
    {
        throw std::runtime_error("Unable to create SVG file");
    }

    // Écrire l'en-tête SVG
    svg_file << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    svg_file << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
    svg_file << "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";

    // Fond blanc
    svg_file << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";

    // Calculer les dimensions de chaque sous-figure
    double subplot_width = static_cast<double>(width) / cols;
    double subplot_height = static_cast<double>(height) / rows;

    // Exporter chaque figure
    for (size_t fig_idx = 0; fig_idx < figures.size(); ++fig_idx)
    {
        const Figure &fig = figures[fig_idx];

        // Calculer la position de la figure
        unsigned int row = fig_idx / cols;
        unsigned int col = fig_idx % cols;
        double x_offset = col * subplot_width;
        double y_offset = row * subplot_height;

        // Définir la taille effective de la figure en tenant compte de equal_axes si nécessaire
        double effective_width = subplot_width;
        double effective_height = subplot_height;
        double margin = 50.0;

        if (fig.equal_axes || fig.is_polar)
        {
            double min_size = std::min(effective_width, effective_height);
            effective_width = min_size;
            effective_height = min_size;
            // Ajuster les offsets pour centrer la figure
            x_offset += (subplot_width - effective_width) / 2.0;
            y_offset += (subplot_height - effective_height) / 2.0;
        }

        // Créer un groupe pour cette figure avec transformation appropriée
        svg_file << "<g transform=\"translate(" << x_offset << "," << y_offset << ")\">\n";

        // Ajouter une bordure autour de la figure
        svg_file << "<rect x=\"0\" y=\"0\" width=\"" << effective_width << "\" height=\"" << effective_height
                 << "\" fill=\"none\" stroke=\"black\" stroke-width=\"1\"/>\n";

        // Dimensions internes de la figure (zone de tracé)
        double graph_width = effective_width - 2 * margin;
        double graph_height = effective_height - 2 * margin;

        // Dessiner les grilles avant les axes
        if (fig.show_major_grid || fig.show_minor_grid)
        {
            if (fig.is_polar)
            {
                export_svg_polar_grid(fig, svg_file, margin, margin, graph_width, graph_height);
            }
            else
            {
                export_svg_grid(fig, svg_file, margin, margin, graph_width, graph_height);
            }
        }

        // Dessiner les axes X et Y seulement pour les graphes non-polaires
        if (!fig.is_polar)
        {
            // Axe X
            double x_axis_y = margin + graph_height - (0 - fig.ymin) / (fig.ymax - fig.ymin) * graph_height;
            x_axis_y = std::min(x_axis_y, margin + graph_height);
            x_axis_y = std::max(x_axis_y, margin);

            svg_file << "<line x1=\"" << margin << "\" y1=\"" << x_axis_y
                     << "\" x2=\"" << (margin + graph_width) << "\" y2=\"" << x_axis_y
                     << "\" stroke=\"black\" stroke-width=\"1.5\"/>\n";

            // Axe Y
            double y_axis_x = margin + (0 - fig.xmin) / (fig.xmax - fig.xmin) * graph_width;
            y_axis_x = std::min(y_axis_x, margin + graph_width);
            y_axis_x = std::max(y_axis_x, margin);

            svg_file << "<line x1=\"" << y_axis_x << "\" y1=\"" << margin
                     << "\" x2=\"" << y_axis_x << "\" y2=\"" << (margin + graph_height)
                     << "\" stroke=\"black\" stroke-width=\"1.5\"/>\n";

            // Dessiner les graduations sur l'axe X
            const int numTicksX = 5;
            for (int i = 0; i <= numTicksX; ++i)
            {
                double x = fig.xmin + (fig.xmax - fig.xmin) * i / numTicksX;
                double sx = margin + (x - fig.xmin) / (fig.xmax - fig.xmin) * graph_width;

                // Trait de graduation
                svg_file << "<line x1=\"" << sx << "\" y1=\"" << x_axis_y
                         << "\" x2=\"" << sx << "\" y2=\"" << (x_axis_y + 5)
                         << "\" stroke=\"black\" stroke-width=\"1\"/>\n";

                // Texte de la graduation
                std::string tickText;
                if (std::abs(x) < 0.01)
                {
                    tickText = "0";
                }
                else if (std::abs(x) < 10)
                {
                    char buffer[10];
                    std::snprintf(buffer, sizeof(buffer), "%.1f", x);
                    tickText = buffer;
                }
                else
                {
                    tickText = std::to_string(static_cast<int>(x));
                }

                svg_file << "<text x=\"" << sx << "\" y=\"" << (x_axis_y + 20)
                         << "\" text-anchor=\"middle\" font-family=\"Arial\" font-size=\"12\">"
                         << tickText << "</text>\n";
            }

            // Dessiner les graduations sur l'axe Y
            const int numTicksY = 5;
            for (int i = 0; i <= numTicksY; ++i)
            {
                double y = fig.ymin + (fig.ymax - fig.ymin) * i / numTicksY;
                double sy = margin + graph_height - (y - fig.ymin) / (fig.ymax - fig.ymin) * graph_height;

                // Trait de graduation
                svg_file << "<line x1=\"" << y_axis_x << "\" y1=\"" << sy
                         << "\" x2=\"" << (y_axis_x - 5) << "\" y2=\"" << sy
                         << "\" stroke=\"black\" stroke-width=\"1\"/>\n";

                // Texte de la graduation
                std::string tickText;
                if (std::abs(y) < 0.01)
                {
                    tickText = "0";
                }
                else if (std::abs(y) < 10)
                {
                    char buffer[10];
                    std::snprintf(buffer, sizeof(buffer), "%.1f", y);
                    tickText = buffer;
                }
                else
                {
                    tickText = std::to_string(static_cast<int>(y));
                }

                svg_file << "<text x=\"" << (y_axis_x - 15) << "\" y=\"" << (sy + 5)
                         << "\" text-anchor=\"end\" font-family=\"Arial\" font-size=\"12\">"
                         << tickText << "</text>\n";
            }
        }

        // Dessiner les courbes
        for (size_t i = 0; i < fig.curves.size(); ++i)
        {
            if (i < fig.curve_types.size())
            {
                const auto &curve = fig.curves[i];
                const auto &curve_type = fig.curve_types[i];

                if (curve_type == "2D" || curve_type == "POLAR")
                {
                    export_svg_curve(fig, curve, svg_file, margin, margin, graph_width, graph_height);
                }
                else if (curve_type == "HIST")
                {
                    export_svg_histogram(fig, curve, svg_file, margin, margin, graph_width, graph_height);
                }
                else if (curve_type == "TEXT")
                {
                    // Gérer le texte dans les figures
                    if (!curve.text_content.empty() && !curve.x.empty() && !curve.y.empty())
                    {
                        double sx = margin + (curve.x[0] - fig.xmin) / (fig.xmax - fig.xmin) * graph_width;
                        double sy = margin + graph_height - (curve.y[0] - fig.ymin) / (fig.ymax - fig.ymin) * graph_height;

                        svg_file << "<text x=\"" << sx << "\" y=\"" << sy
                                 << "\" text-anchor=\"middle\" font-family=\"Arial\" font-size=\""
                                 << (curve.style.thickness > 0 ? curve.style.thickness * 6 : 12)
                                 << "\" fill=\"" << color_to_svg(curve.style.color) << "\">"
                                 << curve.text_content << "</text>\n";
                    }
                }
                else if (curve_type == "ARROW_HEAD")
                {
                    // Gérer les flèches
                    if (curve.x.size() >= 2 && curve.y.size() >= 2)
                    {
                        double x1 = margin + (curve.x[0] - fig.xmin) / (fig.xmax - fig.xmin) * graph_width;
                        double y1 = margin + graph_height - (curve.y[0] - fig.ymin) / (fig.ymax - fig.ymin) * graph_height;
                        double x2 = margin + (curve.x[1] - fig.xmin) / (fig.xmax - fig.xmin) * graph_width;
                        double y2 = margin + graph_height - (curve.y[1] - fig.ymin) / (fig.ymax - fig.ymin) * graph_height;

                        // Dessiner la ligne de la flèche
                        svg_file << "<line x1=\"" << x1 << "\" y1=\"" << y1
                                 << "\" x2=\"" << x2 << "\" y2=\"" << y2
                                 << "\" stroke=\"" << color_to_svg(curve.style.color)
                                 << "\" stroke-width=\"" << curve.style.thickness << "\"/>\n";

                        // Dessiner la tête de flèche
                        double angle = std::atan2(y2 - y1, x2 - x1);
                        double headSize = curve.head_size * 0.5;
                        double headAngle = 30.0 * M_PI / 180.0; // 30 degrés en radians

                        double x3 = x2 - headSize * std::cos(angle - headAngle);
                        double y3 = y2 - headSize * std::sin(angle - headAngle);
                        double x4 = x2 - headSize * std::cos(angle + headAngle);
                        double y4 = y2 - headSize * std::sin(angle + headAngle);

                        svg_file << "<polygon points=\""
                                 << x2 << "," << y2 << " "
                                 << x3 << "," << y3 << " "
                                 << x4 << "," << y4
                                 << "\" fill=\"" << color_to_svg(curve.style.color) << "\"/>\n";
                    }
                }
            }
        }

        // Ajouter le titre et les étiquettes des axes
        if (!fig.title.empty())
        {
            svg_file << "<text x=\"" << (effective_width / 2) << "\" y=\"" << (margin / 2)
                     << "\" text-anchor=\"middle\" font-family=\"Arial\" font-size=\"18\" font-weight=\"bold\">"
                     << fig.title << "</text>\n";
        }

        if (!fig.xlabel.empty())
        {
            svg_file << "<text x=\"" << (effective_width / 2) << "\" y=\"" << (effective_height - 10)
                     << "\" text-anchor=\"middle\" font-family=\"Arial\" font-size=\"14\">"
                     << fig.xlabel << "</text>\n";
        }

        if (!fig.ylabel.empty())
        {
            svg_file << "<text x=\"" << (margin / 3) << "\" y=\"" << (effective_height / 2)
                     << "\" text-anchor=\"middle\" font-family=\"Arial\" font-size=\"14\" "
                     << "transform=\"rotate(-90 " << (margin / 3) << "," << (effective_height / 2) << ")\">"
                     << fig.ylabel << "</text>\n";
        }

        // Ajouter la légende si nécessaire
        if (fig.show_leg)
        {
            // Compter combien d'entrées de légende nous avons
            int legend_entries = 0;
            for (const auto &curve : fig.curves)
            {
                if (!curve.style.legend.empty())
                {
                    legend_entries++;
                }
            }

            if (legend_entries > 0)
            {
                double legendWidth = 150;
                double legendHeight = legend_entries * 20 + 10;
                double legendX, legendY;

                // Positionner la légende selon la configuration
                if (fig.legend_position == "top-right")
                {
                    legendX = effective_width - legendWidth - 10;
                    legendY = margin + 10;
                }
                else if (fig.legend_position == "top-left")
                {
                    legendX = margin + 10;
                    legendY = margin + 10;
                }
                else if (fig.legend_position == "bottom-right")
                {
                    legendX = effective_width - legendWidth - 10;
                    legendY = effective_height - legendHeight - margin;
                }
                else if (fig.legend_position == "bottom-left")
                {
                    legendX = margin + 10;
                    legendY = effective_height - legendHeight - margin;
                }
                else
                {
                    // Position par défaut: top-right
                    legendX = effective_width - legendWidth - 10;
                    legendY = margin + 10;
                }

                // Dessiner le fond de la légende
                svg_file << "<rect x=\"" << legendX << "\" y=\"" << legendY
                         << "\" width=\"" << legendWidth << "\" height=\"" << legendHeight
                         << "\" fill=\"white\" fill-opacity=\"0.8\" stroke=\"black\" stroke-width=\"1\"/>\n";

                // Ajouter chaque entrée de légende
                double currentY = legendY + 15;
                for (const auto &curve : fig.curves)
                {
                    if (!curve.style.legend.empty())
                    {
                        // Dessiner l'exemple de style (ligne et symbole)
                        if (curve.style.line_style == "solid")
                        {
                            svg_file << "<line x1=\"" << (legendX + 10) << "\" y1=\"" << currentY
                                     << "\" x2=\"" << (legendX + 40) << "\" y2=\"" << currentY
                                     << "\" stroke=\"" << color_to_svg(curve.style.color)
                                     << "\" stroke-width=\"" << curve.style.thickness;

                            // Pas de dashed/dotted pour les lignes solides
                            svg_file << "\"/>\n";
                        }
                        else if (curve.style.line_style == "dashed")
                        {
                            svg_file << "<line x1=\"" << (legendX + 10) << "\" y1=\"" << currentY
                                     << "\" x2=\"" << (legendX + 40) << "\" y2=\"" << currentY
                                     << "\" stroke=\"" << color_to_svg(curve.style.color)
                                     << "\" stroke-width=\"" << curve.style.thickness
                                     << "\" stroke-dasharray=\"" << (5 * curve.style.thickness) << ","
                                     << (3 * curve.style.thickness) << "\"/>\n";
                        }

                        // Ajouter le symbole si nécessaire
                        if (curve.style.symbol_type == "circle")
                        {
                            svg_file << "<circle cx=\"" << (legendX + 25) << "\" cy=\"" << currentY
                                     << "\" r=\"" << (curve.style.symbol_size / 2)
                                     << "\" fill=\"" << color_to_svg(curve.style.color)
                                     << "\" stroke=\"black\" stroke-width=\"1\"/>\n";
                        }
                        else if (curve.style.symbol_type == "square")
                        {
                            double halfSize = curve.style.symbol_size / 2;
                            svg_file << "<rect x=\"" << (legendX + 25 - halfSize) << "\" y=\"" << (currentY - halfSize)
                                     << "\" width=\"" << curve.style.symbol_size << "\" height=\"" << curve.style.symbol_size
                                     << "\" fill=\"" << color_to_svg(curve.style.color)
                                     << "\" stroke=\"black\" stroke-width=\"1\"/>\n";
                        }

                        // Ajouter le texte de la légende
                        svg_file << "<text x=\"" << (legendX + 45) << "\" y=\"" << (currentY + 5)
                                 << "\" font-family=\"Arial\" font-size=\"12\">"
                                 << curve.style.legend << "</text>\n";

                        currentY += 20;
                    }
                }
            }
        }

        // Fermer le groupe
        svg_file << "</g>\n";
    }

    // Fermeture de la balise SVG
    svg_file << "</svg>\n";
    svg_file.close();

    std::cout << "SVG vectoriel exporté vers: " << filename << " (Dimensions: " << width << "x" << height << ")" << std::endl;
}

void PlotGen::render()
{
    texture.clear(sf::Color::White);

    // Go through all subplots
    for (unsigned int row = 0; row < rows; ++row)
    {
        for (unsigned int col = 0; col < cols; ++col)
        {
            Figure &fig = figures[row * cols + col];

            // Define the view for this subplot
            double subplot_width = static_cast<double>(width) / cols;
            double subplot_height = static_cast<double>(height) / rows;

            // For polar graphs, ensure dimensions are square
            if (fig.is_polar)
            {
                double min_size = std::min(subplot_width, subplot_height);
                double x_offset = (subplot_width - min_size) / 2.0f;
                double y_offset = (subplot_height - min_size) / 2.0f;

                sf::View view(sf::FloatRect(0, 0, min_size, min_size));
                view.setViewport(sf::FloatRect(
                    static_cast<double>(col) / cols + x_offset / width,
                    static_cast<double>(row) / rows + y_offset / height,
                    min_size / width,
                    min_size / height));
                texture.setView(view);
            }
            else if (fig.equal_axes)
            {
                double min_size = std::min(subplot_width, subplot_height);
                double x_offset = (subplot_width - min_size) / 2.0f;
                double y_offset = (subplot_height - min_size) / 2.0f;

                sf::View view(sf::FloatRect(0, 0, min_size, min_size));
                view.setViewport(sf::FloatRect(
                    static_cast<double>(col) / cols + x_offset / width,
                    static_cast<double>(row) / rows + y_offset / height,
                    min_size / width,
                    min_size / height));
                texture.setView(view);
            }
            else
            {
                sf::View view(sf::FloatRect(0, 0, subplot_width, subplot_height));
                view.setViewport(sf::FloatRect(
                    static_cast<double>(col) / cols,
                    static_cast<double>(row) / rows,
                    1.0f / cols,
                    1.0f / rows));
                texture.setView(view);
            }

            // Draw the subplot frame
            sf::RectangleShape frame;
            if (fig.is_polar || fig.equal_axes)
            {
                double min_size = std::min(subplot_width, subplot_height);
                frame.setSize(sf::Vector2f(min_size, min_size));
            }
            else
            {
                frame.setSize(sf::Vector2f(subplot_width, subplot_height));
            }
            frame.setFillColor(sf::Color::Transparent);
            frame.setOutlineColor(sf::Color::Black);
            frame.setOutlineThickness(1.0f);
            texture.draw(frame);

            // Draw axes
            draw_axes(fig, fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_width,
                      fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_height);

            // Draw curves
            for (size_t i = 0; i < fig.curves.size(); ++i)
            {
                if (i < fig.curve_types.size())
                {
                    if (fig.curve_types[i] == "2D")
                        draw_curve(fig, fig.curves[i], fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_width,
                                   fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_height);
                    else if (fig.curve_types[i] == "HIST")
                        draw_histogram(fig, fig.curves[i], fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_width,
                                       fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_height);
                    else if (fig.curve_types[i] == "POLAR")
                        draw_curve(fig, fig.curves[i], fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_width,
                                   fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_height);
                    else if (fig.curve_types[i] == "TEXT")
                        draw_text(fig, fig.curves[i], fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_width,
                                  fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_height);
                    else if (fig.curve_types[i] == "ARROW_HEAD")
                        draw_arrow_head(fig, fig.curves[i], fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_width,
                                        fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_height);
                }
            }

            // Draw text (title, axes, legend)
            draw_text(fig, fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_width,
                      fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_height);
        }
    }

    // Restore default view
    texture.setView(texture.getDefaultView());
    texture.display();
}

void PlotGen::draw_axes(const Figure &fig, double w, double h)
{
    // double margin = 50.0f;

    // Draw grids first (to be in the background)
    if (fig.show_major_grid || fig.show_minor_grid)
    {
        draw_grid(fig, w, h);
    }

    // X axis
    sf::VertexArray xAxis(sf::Lines, 2);
    xAxis[0].position = to_screen(fig, fig.xmin, 0, w, h);
    xAxis[1].position = to_screen(fig, fig.xmax, 0, w, h);
    xAxis[0].color = sf::Color::Black;
    xAxis[1].color = sf::Color::Black;

    // Y axis
    sf::VertexArray yAxis(sf::Lines, 2);
    yAxis[0].position = to_screen(fig, 0, fig.ymin, w, h);
    yAxis[1].position = to_screen(fig, 0, fig.ymax, w, h);
    yAxis[0].color = sf::Color::Black;
    yAxis[1].color = sf::Color::Black;

    texture.draw(xAxis);
    texture.draw(yAxis);

    // Ticks on X axis
    const int numTicksX = 5;
    for (int i = 0; i <= numTicksX; ++i)
    {
        double x = fig.xmin + (fig.xmax - fig.xmin) * i / numTicksX;
        sf::VertexArray tick(sf::Lines, 2);
        tick[0].position = to_screen(fig, x, 0, w, h);
        tick[1].position = to_screen(fig, x, 0, w, h) + sf::Vector2f(0, 5);
        tick[0].color = sf::Color::Black;
        tick[1].color = sf::Color::Black;
        texture.draw(tick);

        // Tick value
        sf::Text tickLabel;
        tickLabel.setFont(font);
        tickLabel.setCharacterSize(14);
        tickLabel.setFillColor(sf::Color::Black);

        // Use appropriate decimal precision
        std::string tickText;
        if (std::abs(x) < 0.01)
        {
            tickText = "0";
        }
        else if (std::abs(x) < 10)
        {
            char buffer[10];
            std::snprintf(buffer, sizeof(buffer), "%.1f", x);
            tickText = buffer;
        }
        else
        {
            tickText = std::to_string(static_cast<int>(x));
        }

        tickLabel.setString(sf::String::fromUtf8(tickText.begin(), tickText.end()));
        tickLabel.setPosition(to_screen(fig, x, 0, w, h) + sf::Vector2f(-10, 8));
        texture.draw(tickLabel);
    }

    // Ticks on Y axis
    const int numTicksY = 5;
    for (int i = 0; i <= numTicksY; ++i)
    {
        double y = fig.ymin + (fig.ymax - fig.ymin) * i / numTicksY;
        sf::VertexArray tick(sf::Lines, 2);
        tick[0].position = to_screen(fig, 0, y, w, h);
        tick[1].position = to_screen(fig, 0, y, w, h) + sf::Vector2f(-5, 0);
        tick[0].color = sf::Color::Black;
        tick[1].color = sf::Color::Black;
        texture.draw(tick);

        // Tick value
        sf::Text tickLabel;
        tickLabel.setFont(font);
        tickLabel.setCharacterSize(14);
        tickLabel.setFillColor(sf::Color::Black);

        // Use appropriate decimal precision
        std::string tickText;
        if (std::abs(y) < 0.01)
        {
            tickText = "0";
        }
        else if (std::abs(y) < 10)
        {
            char buffer[10];
            std::snprintf(buffer, sizeof(buffer), "%.1f", y);
            tickText = buffer;
        }
        else
        {
            tickText = std::to_string(static_cast<int>(y));
        }

        tickLabel.setString(sf::String::fromUtf8(tickText.begin(), tickText.end()));
        tickLabel.setPosition(to_screen(fig, 0, y, w, h) + sf::Vector2f(-30, -10));
        texture.draw(tickLabel);
    }
}

void PlotGen::draw_grid(const Figure &fig, double w, double h)
{
    if (fig.is_polar)
    {
        draw_polar_grid(fig, w, h);
        return;
    }

    // Number of major divisions
    const int numTicksX = 5;
    const int numTicksY = 5;

    // Number of minor subdivisions (between each major line)
    const int numMinorSubdivisions = 4;

    // Draw major grid
    if (fig.show_major_grid)
    {
        sf::VertexArray majorGrid(sf::Lines);

        // Major vertical lines
        for (int i = 0; i <= numTicksX; ++i)
        {
            double x = fig.xmin + (fig.xmax - fig.xmin) * i / numTicksX;
            sf::Vector2f top = to_screen(fig, x, fig.ymin, w, h);
            sf::Vector2f bottom = to_screen(fig, x, fig.ymax, w, h);

            majorGrid.append(sf::Vertex(top, fig.major_grid_color));
            majorGrid.append(sf::Vertex(bottom, fig.major_grid_color));
        }

        // Major horizontal lines
        for (int i = 0; i <= numTicksY; ++i)
        {
            double y = fig.ymin + (fig.ymax - fig.ymin) * i / numTicksY;
            sf::Vector2f left = to_screen(fig, fig.xmin, y, w, h);
            sf::Vector2f right = to_screen(fig, fig.xmax, y, w, h);

            majorGrid.append(sf::Vertex(left, fig.major_grid_color));
            majorGrid.append(sf::Vertex(right, fig.major_grid_color));
        }

        texture.draw(majorGrid);
    }

    // Draw minor grid
    if (fig.show_minor_grid)
    {
        sf::VertexArray minorGrid(sf::Lines);

        // Minor vertical lines
        for (int i = 0; i < numTicksX; ++i)
        {
            double x_start = fig.xmin + (fig.xmax - fig.xmin) * i / numTicksX;
            double x_step = (fig.xmax - fig.xmin) / (numTicksX * numMinorSubdivisions);

            for (int j = 1; j < numMinorSubdivisions; ++j)
            {
                double x = x_start + j * x_step;
                sf::Vector2f top = to_screen(fig, x, fig.ymin, w, h);
                sf::Vector2f bottom = to_screen(fig, x, fig.ymax, w, h);

                minorGrid.append(sf::Vertex(top, fig.minor_grid_color));
                minorGrid.append(sf::Vertex(bottom, fig.minor_grid_color));
            }
        }

        // Minor horizontal lines
        for (int i = 0; i < numTicksY; ++i)
        {
            double y_start = fig.ymin + (fig.ymax - fig.ymin) * i / numTicksY;
            double y_step = (fig.ymax - fig.ymin) / (numTicksY * numMinorSubdivisions);

            for (int j = 1; j < numMinorSubdivisions; ++j)
            {
                double y = y_start + j * y_step;
                sf::Vector2f left = to_screen(fig, fig.xmin, y, w, h);
                sf::Vector2f right = to_screen(fig, fig.xmax, y, w, h);

                minorGrid.append(sf::Vertex(left, fig.minor_grid_color));
                minorGrid.append(sf::Vertex(right, fig.minor_grid_color));
            }
        }

        texture.draw(minorGrid);
    }
}

void PlotGen::draw_polar_grid(const Figure &fig, double w, double h)
{
    // Center of the polar grid
    sf::Vector2f center = to_screen(fig, 0, 0, w, h);

    // Maximum radius for circles (in pixels)
    double max_radius = std::min(w, h) / 2 - 50;

    // Maximum radius of data (in units)
    double max_r = std::max(std::abs(fig.xmax), std::abs(fig.ymax));

    // Number of concentric circles for major grid
    const int numCircles = 5;

    // Number of rays for major grid (one ray every 30 degrees)
    const int numRays = 12;

    // Draw concentric circles (major grid)
    if (fig.show_major_grid)
    {
        // Concentric circles
        for (int i = 1; i <= numCircles; ++i)
        {
            double radius = max_radius * i / numCircles;
            double r_value = max_r * i / numCircles;

            sf::CircleShape circle(radius);
            circle.setOrigin(radius, radius);
            circle.setPosition(center);
            circle.setFillColor(sf::Color::Transparent);
            circle.setOutlineColor(fig.major_grid_color);
            circle.setOutlineThickness(1.0f);
            texture.draw(circle);

            // Add radius labels with one decimal place
            sf::Text rLabel;
            rLabel.setFont(font);
            rLabel.setCharacterSize(10);
            rLabel.setFillColor(sf::Color::Black);

            // Format radius value with one decimal place
            char r_buffer[10];
            std::snprintf(r_buffer, sizeof(r_buffer), "%.1f", r_value);
            rLabel.setString(r_buffer);

            sf::FloatRect textRect = rLabel.getLocalBounds();
            rLabel.setPosition(center.x + radius * std::cos(3.14f / 4) - textRect.width / 2,
                               center.y - radius * std::sin(3.14f / 4) - textRect.height / 2);
            texture.draw(rLabel);
        }

        // Rays from center
        sf::VertexArray rays(sf::Lines);
        for (int i = 0; i < numRays; ++i)
        {
            double angle = 2 * M_PI * i / numRays;
            sf::Vector2f end(center.x + max_radius * std::cos(angle),
                             center.y - max_radius * std::sin(angle));

            rays.append(sf::Vertex(center, fig.major_grid_color));
            rays.append(sf::Vertex(end, fig.major_grid_color));

            // Add angle labels (in degrees) with one decimal place
            sf::Text angleLabel;
            angleLabel.setFont(font);
            angleLabel.setCharacterSize(12);
            angleLabel.setFillColor(sf::Color::Black);

            // Calculate degrees and format with one decimal place
            double degrees = angle * 180 / M_PI;
            if (degrees >= 360)
                degrees -= 360;

            char angle_buffer[15];
            std::snprintf(angle_buffer, sizeof(angle_buffer), "%.1f°", degrees);
            angleLabel.setString(sf::String::fromUtf8(angle_buffer, angle_buffer + strlen(angle_buffer)));

            sf::FloatRect textRect = angleLabel.getLocalBounds();

            // Position the label slightly beyond the end of the ray
            sf::Vector2f labelPos(center.x + (max_radius + 10) * std::cos(angle) - textRect.width / 2,
                                  center.y - (max_radius + 10) * std::sin(angle) - textRect.height / 2);
            angleLabel.setPosition(labelPos);
            texture.draw(angleLabel);
        }
        texture.draw(rays);
    }

    // Draw minor grid
    if (fig.show_minor_grid)
    {
        // Minor concentric circles between major ones
        const int numMinorCircles = 4;
        for (int i = 0; i < numCircles; ++i)
        {
            double radiusStart = max_radius * i / numCircles;
            double radiusStep = max_radius / numCircles / numMinorCircles;

            for (int j = 1; j < numMinorCircles; ++j)
            {
                double radius = radiusStart + j * radiusStep;

                sf::CircleShape circle(radius);
                circle.setOrigin(radius, radius);
                circle.setPosition(center);
                circle.setFillColor(sf::Color::Transparent);
                circle.setOutlineColor(fig.minor_grid_color);
                circle.setOutlineThickness(1.0f);
                texture.draw(circle);
            }
        }

        // Minor rays between major ones
        sf::VertexArray minorRays(sf::Lines);
        const int numMinorRays = numRays * 2; // One ray every 15 degrees

        for (int i = 0; i < numMinorRays; ++i)
        {
            // Skip rays that are already in the major grid
            if (i % 2 == 0)
                continue;

            double angle = 2 * M_PI * i / numMinorRays;
            sf::Vector2f end(center.x + max_radius * std::cos(angle),
                             center.y - max_radius * std::sin(angle));

            minorRays.append(sf::Vertex(center, fig.minor_grid_color));
            minorRays.append(sf::Vertex(end, fig.minor_grid_color));
        }
        texture.draw(minorRays);
    }
}

void PlotGen::draw_curve(const Figure &fig, const Figure::Curve &curve, double w, double h)
{
    if (curve.x.empty() || curve.y.empty())
        return;

    // Appliquer l'épaisseur des lignes en utilisant les vertex arrays avec triangles
    // pour les lignes de plus d'un pixel d'épaisseur
    float thickness = static_cast<float>(curve.style.thickness);

    // Collecter tous les points pour dessiner les symboles après les lignes
    std::vector<sf::Vector2f> symbolPoints;

    // Convertir tous les points de la courbe en coordonnées d'écran
    for (size_t i = 0; i < curve.x.size(); ++i)
    {
        symbolPoints.push_back(to_screen(fig, curve.x[i], curve.y[i], w, h));
    }

    if (curve.style.line_style == "solid")
    {
        if (thickness <= 1.0f)
        {
            // Pour les lignes fines, utiliser LineStrip (plus efficace)
            sf::VertexArray line(sf::LineStrip);
            for (size_t i = 0; i < curve.x.size(); ++i)
            {
                sf::Vector2f position = symbolPoints[i];
                sf::Vertex point(position, curve.style.color);
                line.append(point);
            }
            texture.draw(line);
        }
        else
        {
            // Pour les lignes épaisses, utiliser des triangles
            if (curve.x.size() > 1)
            {
                sf::VertexArray thickLine(sf::Triangles);

                for (size_t i = 0; i < curve.x.size() - 1; ++i)
                {
                    sf::Vector2f p1 = symbolPoints[i];
                    sf::Vector2f p2 = symbolPoints[i + 1];

                    // Calculer le vecteur normalisé perpendiculaire à la ligne
                    sf::Vector2f direction = p2 - p1;
                    sf::Vector2f unitDirection;
                    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                    if (length > 0)
                    {
                        unitDirection = direction / length;
                        sf::Vector2f unitPerpendicular(-unitDirection.y, unitDirection.x);

                        // Calculer les points pour former un rectangle (deux triangles)
                        sf::Vector2f offset = unitPerpendicular * (thickness / 2.0f);

                        sf::Vertex v1(p1 + offset, curve.style.color);
                        sf::Vertex v2(p2 + offset, curve.style.color);
                        sf::Vertex v3(p2 - offset, curve.style.color);
                        sf::Vertex v4(p1 - offset, curve.style.color);

                        // Premier triangle
                        thickLine.append(v1);
                        thickLine.append(v2);
                        thickLine.append(v3);

                        // Second triangle
                        thickLine.append(v1);
                        thickLine.append(v3);
                        thickLine.append(v4);
                    }
                }

                texture.draw(thickLine);
            }
        }
    }
    else if (curve.style.line_style == "dashed")
    {
        // Pour les lignes pointillées
        if (thickness <= 1.0f)
        {
            // Lignes fines
            sf::VertexArray line(sf::Lines);
            for (size_t i = 0; i < curve.x.size() - 1; i += 2)
            {
                if (i + 1 < symbolPoints.size())
                {
                    sf::Vector2f p1 = symbolPoints[i];
                    sf::Vector2f p2 = symbolPoints[i + 1];

                    line.append(sf::Vertex(p1, curve.style.color));
                    line.append(sf::Vertex(p2, curve.style.color));
                }
            }
            texture.draw(line);
        }
        else
        {
            // Lignes épaisses pointillées
            if (curve.x.size() > 1)
            {
                sf::VertexArray thickLine(sf::Triangles);

                for (size_t i = 0; i < curve.x.size() - 1; i += 2)
                {
                    if (i + 1 >= symbolPoints.size())
                        continue;

                    sf::Vector2f p1 = symbolPoints[i];
                    sf::Vector2f p2 = symbolPoints[i + 1];

                    // Même logique que pour les lignes pleines
                    sf::Vector2f direction = p2 - p1;
                    sf::Vector2f unitDirection;
                    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                    if (length > 0)
                    {
                        unitDirection = direction / length;
                        sf::Vector2f unitPerpendicular(-unitDirection.y, unitDirection.x);

                        sf::Vector2f offset = unitPerpendicular * (thickness / 2.0f);

                        sf::Vertex v1(p1 + offset, curve.style.color);
                        sf::Vertex v2(p2 + offset, curve.style.color);
                        sf::Vertex v3(p2 - offset, curve.style.color);
                        sf::Vertex v4(p1 - offset, curve.style.color);

                        thickLine.append(v1);
                        thickLine.append(v2);
                        thickLine.append(v3);

                        thickLine.append(v1);
                        thickLine.append(v3);
                        thickLine.append(v4);
                    }
                }

                texture.draw(thickLine);
            }
        }
    }
    else if (curve.style.line_style == "points")
    { // points
        sf::VertexArray line(sf::Points);
        for (const auto &position : symbolPoints)
        {
            sf::Vertex point(position, curve.style.color);
            line.append(point);
        }
        texture.draw(line);
    }

    // Dessiner les symboles après avoir dessiné toutes les lignes
    // pour s'assurer qu'ils sont visibles par-dessus les lignes
    if (curve.style.symbol_type != "none")
    {
        for (const auto &position : symbolPoints)
        {
            draw_symbol(position, curve.style.symbol_type, curve.style.symbol_size, curve.style.color);
        }
    }
}

void PlotGen::draw_histogram(const Figure &fig, const Figure::Curve &curve, double w, double h)
{
    if (curve.x.empty() || curve.y.empty())
        return;

    double maxY = *std::max_element(curve.y.begin(), curve.y.end());
    if (maxY <= 0)
        maxY = 1.0f; // Avoid division by zero

    // Calculate bin width based on data
    double bin_width = 0;
    if (curve.x.size() > 1)
    {
        bin_width = curve.x[1] - curve.x[0];
    }
    else
    {
        bin_width = (fig.xmax - fig.xmin) / 20.0f;
    }

    // Calculate bar width to properly fill the space
    double bar_width = bin_width * w / (fig.xmax - fig.xmin);

    // Ensure bars aren't too wide
    bar_width *= curve.bar_width_ratio; // Use custom width ratio

    for (size_t i = 0; i < curve.x.size(); ++i)
    {
        sf::RectangleShape bar;
        double bar_height = curve.y[i] * (h - 100) / (fig.ymax - fig.ymin); // Adjust for margins

        // Correct bar position, centered on x value
        sf::Vector2f position = to_screen(fig, curve.x[i], 0, w, h);
        position.x -= bar_width / 2; // Center bar on x value

        bar.setSize(sf::Vector2f(bar_width, -bar_height)); // negative to draw upward
        bar.setPosition(position);
        bar.setFillColor(curve.style.color);
        bar.setOutlineColor(sf::Color::Black);
        bar.setOutlineThickness(1.0f);

        texture.draw(bar);
    }
}

sf::Color PlotGen::getColorFromHeight(double height)
{
    // Create a color gradient blue->cyan->green->yellow->red
    if (height < 0.25f)
    {
        // Blue to cyan
        return sf::Color(0, static_cast<sf::Uint8>(255 * height * 4), 255);
    }
    else if (height < 0.5f)
    {
        // Cyan to green
        return sf::Color(0, 255, static_cast<sf::Uint8>(255 * (1 - (height - 0.25f) * 4)));
    }
    else if (height < 0.75f)
    {
        // Green to yellow
        return sf::Color(static_cast<sf::Uint8>(255 * (height - 0.5f) * 4), 255, 0);
    }
    else
    {
        // Yellow to red
        return sf::Color(255, static_cast<sf::Uint8>(std::max(0, static_cast<int>(255 * (1 - (height - 0.75f) * 4)))), 0);
    }
}

sf::Vector2f PlotGen::to_screen(const Figure &fig, double x, double y, double w, double h) const
{
    double margin = 50.0f;

    // Calculate screen coordinates
    double sx = margin + (x - fig.xmin) / (fig.xmax - fig.xmin) * (w - 2 * margin);
    double sy = h - margin - (y - fig.ymin) / (fig.ymax - fig.ymin) * (h - 2 * margin);

    // Ensure points stay within graph boundaries
    sx = std::max(margin, std::min(w - margin, sx));
    sy = std::max(margin, std::min(h - margin, sy));

    return sf::Vector2f(static_cast<float>(sx), static_cast<float>(sy));
}

void PlotGen::draw_text(const Figure &fig, double w, double h)
{
    sf::Text text;
    text.setFont(font);
    text.setFillColor(sf::Color::Black);

    // Titre avec une taille de police réduite
    text.setCharacterSize(18);
    text.setString(sf::String::fromUtf8(fig.title.begin(), fig.title.end()));
    sf::FloatRect textRect = text.getLocalBounds();

    // Positionner le titre en dehors de la zone de dessin
    double margin = 50.0f;
    text.setPosition(w / 2 - textRect.width / 2, margin / 2 - textRect.height / 2);
    texture.draw(text);

    // X label avec police plus petite
    text.setCharacterSize(14);
    text.setString(sf::String::fromUtf8(fig.xlabel.begin(), fig.xlabel.end()));
    textRect = text.getLocalBounds();
    text.setPosition(w / 2 - textRect.width / 2, h - 20);
    texture.draw(text);

    // Y label avec police plus petite
    text.setCharacterSize(14);
    text.setString(sf::String::fromUtf8(fig.ylabel.begin(), fig.ylabel.end()));
    textRect = text.getLocalBounds();
    text.setRotation(-90);
    text.setPosition(10, h / 2 + textRect.width / 2);
    texture.draw(text);
    text.setRotation(0);

    // Légende
    if (fig.show_leg && !fig.curves.empty())
    {
        const double max_legend_width = 130.0f; // Largeur maximale du texte de légende
        double padding_x = 10.0f;               // Marge horizontale
        double padding_y = 8.0f;                // Marge verticale

        // Collecter toutes les légendes et calculer les dimensions
        std::vector<std::pair<const Figure::Curve *, std::vector<std::string>>> legend_items;
        double total_legend_height = 0.0f;
        double max_content_width = 0.0f;

        // Première passe: collecter et préparer les légendes
        for (const auto &curve : fig.curves)
        {
            if (!curve.style.legend.empty())
            {
                std::vector<std::string> legend_lines;
                std::string legend_text = curve.style.legend;

                text.setCharacterSize(12);
                text.setString(sf::String::fromUtf8(legend_text.begin(), legend_text.end()));
                sf::FloatRect bounds = text.getLocalBounds();

                // Couper le texte si trop long
                if (bounds.width > max_legend_width)
                {
                    // Découper en mots
                    std::vector<std::string> words;
                    std::string current_word;
                    for (char c : legend_text)
                    {
                        if (c == ' ' || c == '\t' || c == '\n')
                        {
                            if (!current_word.empty())
                            {
                                words.push_back(current_word);
                                current_word.clear();
                            }
                            if (c == '\n')
                                words.push_back("\n");
                        }
                        else
                        {
                            current_word += c;
                        }
                    }
                    if (!current_word.empty())
                        words.push_back(current_word);

                    // Construire des lignes
                    std::string current_line;
                    for (const std::string &word : words)
                    {
                        if (word == "\n")
                        {
                            if (!current_line.empty())
                            {
                                legend_lines.push_back(current_line);
                                current_line.clear();
                            }
                            continue;
                        }

                        std::string test_line = current_line;
                        if (!test_line.empty())
                            test_line += " ";
                        test_line += word;

                        text.setString(sf::String::fromUtf8(test_line.begin(), test_line.end()));
                        bounds = text.getLocalBounds();

                        if (bounds.width <= max_legend_width)
                        {
                            current_line = test_line;
                        }
                        else
                        {
                            if (!current_line.empty())
                            {
                                legend_lines.push_back(current_line);
                            }
                            current_line = word;
                        }
                    }

                    if (!current_line.empty())
                    {
                        legend_lines.push_back(current_line);
                    }
                }
                else
                {
                    legend_lines.push_back(legend_text);
                }

                // Ajouter à notre collection
                legend_items.push_back({&curve, legend_lines});

                // Calculer la hauteur pour cette entrée
                double item_height = 24 + (legend_lines.size() - 1) * 16; // 24px pour la première ligne, 16px pour les suivantes
                total_legend_height += item_height;

                // Calculer la largeur maximale du contenu
                for (const auto &line : legend_lines)
                {
                    text.setString(sf::String::fromUtf8(line.begin(), line.end()));
                    double line_width = text.getLocalBounds().width;
                    max_content_width = std::max(max_content_width, line_width);
                }
            }
        }

        // Si nous avons des éléments à afficher
        if (!legend_items.empty())
        {
            // Calculer la taille exacte et la position du rectangle de la légende
            double legend_width = max_content_width + 40.0f + 2 * padding_x; // 40px pour l'exemple de style
            double legend_height = total_legend_height + 2 * padding_y;

            double legend_x, legend_y;

            // Déterminer la position en fonction de fig.legend_position
            if (fig.legend_position == "top-right")
            {
                legend_x = w - legend_width - padding_x;
                legend_y = padding_y + margin;
            }
            else if (fig.legend_position == "top-left")
            {
                legend_x = margin + padding_x;
                legend_y = padding_y + margin;
            }
            else if (fig.legend_position == "bottom-right")
            {
                legend_x = w - legend_width - padding_x;
                legend_y = h - legend_height - padding_y - margin;
            }
            else if (fig.legend_position == "bottom-left")
            {
                legend_x = margin + padding_x;
                legend_y = h - legend_height - padding_y - margin;
            }
            else if (fig.legend_position == "outside-right")
            {
                // Placer la légende à l'extérieur du graphique à droite
                legend_x = w + padding_x;
                legend_y = margin + padding_y;

                // Ajuster la vue pour que la légende soit visible
                sf::View current_view = texture.getView();
                sf::FloatRect viewport = current_view.getViewport();

                // Élargir la vue pour inclure la légende
                viewport.width += legend_width / w;
                current_view.setViewport(viewport);
                texture.setView(current_view);
            }
            else
            {
                // Position par défaut: top-right
                legend_x = w - legend_width - padding_x;
                legend_y = padding_y + margin;
            }

            // Dessiner l'arrière-plan de la légende
            sf::RectangleShape legendBg;
            legendBg.setSize(sf::Vector2f(legend_width, legend_height));
            legendBg.setPosition(legend_x, legend_y);
            legendBg.setFillColor(sf::Color(255, 255, 255, 220));
            legendBg.setOutlineColor(sf::Color::Black);
            legendBg.setOutlineThickness(1.0f);
            texture.draw(legendBg);

            // Dessiner les éléments de la légende
            double current_y = legend_y + padding_y;

            for (const auto &item : legend_items)
            {
                const Figure::Curve *curve = item.first;
                const std::vector<std::string> &legend_lines = item.second;

                // Dessiner l'exemple de style
                float sample_width = 30.0f;
                // float sample_height = 16.0f;
                float start_x = legend_x + padding_x;
                float mid_y = current_y + 8.0f;

                // Style de ligne
                if (curve->style.line_style == "solid")
                {
                    float thickness = std::max(1.0f, static_cast<float>(curve->style.thickness));
                    sf::RectangleShape line(sf::Vector2f(sample_width, thickness));
                    line.setPosition(start_x, mid_y - thickness / 2);
                    line.setFillColor(curve->style.color);
                    texture.draw(line);
                }
                else if (curve->style.line_style == "dashed")
                {
                    float dash_length = 6.0f;
                    float thickness = std::max(1.0f, static_cast<float>(curve->style.thickness));
                    for (int i = 0; i < 3; i++)
                    {
                        sf::RectangleShape dash(sf::Vector2f(dash_length, thickness));
                        dash.setPosition(start_x + i * 2 * dash_length, mid_y - thickness / 2);
                        dash.setFillColor(curve->style.color);
                        texture.draw(dash);
                    }
                }

                // Symbole
                if (curve->style.symbol_type != "none")
                {
                    sf::Vector2f symbol_pos(start_x + sample_width / 2, mid_y);
                    draw_symbol(symbol_pos, curve->style.symbol_type, curve->style.symbol_size, curve->style.color);
                }

                // Texte de la légende
                text.setCharacterSize(12);
                float text_y = current_y;
                for (const auto &line : legend_lines)
                {
                    text.setString(sf::String::fromUtf8(line.begin(), line.end()));
                    text.setPosition(start_x + sample_width + 10, text_y);
                    texture.draw(text);
                    text_y += 16;
                }

                // Passer à l'élément suivant
                current_y += 24 + (legend_lines.size() - 1) * 16;
            }
        }
    }
}

// Method for drawing a symbol at a given position
void PlotGen::draw_symbol(const sf::Vector2f &position, const std::string &symbol_type, double size, const sf::Color &color)
{
    if (symbol_type == "none")
        return;

    if (symbol_type == "circle")
    {
        sf::CircleShape circle(size / 2);
        circle.setOrigin(size / 2, size / 2);
        circle.setPosition(position);
        circle.setFillColor(color);
        circle.setOutlineColor(sf::Color::Black);
        circle.setOutlineThickness(1.0f);
        texture.draw(circle);
    }
    else if (symbol_type == "square")
    {
        sf::RectangleShape square(sf::Vector2f(size, size));
        square.setOrigin(size / 2, size / 2);
        square.setPosition(position);
        square.setFillColor(color);
        square.setOutlineColor(sf::Color::Black);
        square.setOutlineThickness(1.0f);
        texture.draw(square);
    }
    else if (symbol_type == "triangle")
    {
        sf::CircleShape triangle(size / 2, 3); // triangle = circle with 3 sides
        triangle.setOrigin(size / 2, size / 2);
        triangle.setPosition(position);
        triangle.setFillColor(color);
        triangle.setOutlineColor(sf::Color::Black);
        triangle.setOutlineThickness(1.0f);
        texture.draw(triangle);
    }
    else if (symbol_type == "diamond")
    {
        sf::CircleShape diamond(size / 2, 4); // diamond = circle with 4 sides
        diamond.setOrigin(size / 2, size / 2);
        diamond.setPosition(position);
        diamond.setRotation(45.0f); // 45° rotation to get a diamond
        diamond.setFillColor(color);
        diamond.setOutlineColor(sf::Color::Black);
        diamond.setOutlineThickness(1.0f);
        texture.draw(diamond);
    }
    else if (symbol_type == "star")
    {
        // Create a 5-pointed star
        const int numPoints = 5;
        const double innerRadius = size / 4;
        const double outerRadius = size / 2;

        sf::ConvexShape star;
        star.setPointCount(numPoints * 2);
        star.setOrigin(size / 2, size / 2);
        star.setPosition(position);
        star.setFillColor(color);
        star.setOutlineColor(sf::Color::Black);
        star.setOutlineThickness(1.0f);

        for (int i = 0; i < numPoints * 2; i++)
        {
            double radius = (i % 2 == 0) ? outerRadius : innerRadius;
            double angle = i * 3.14159f / numPoints;
            sf::Vector2f point(radius * std::cos(angle), radius * std::sin(angle));
            star.setPoint(i, point);
        }

        texture.draw(star);
    }
}

// Method for drawing text for a TEXT type curve
void PlotGen::draw_text(const Figure &fig, const Figure::Curve &curve, double w, double h)
{
    if (curve.text_content.empty() || curve.x.empty() || curve.y.empty())
        return;

    // Get screen position for the text
    sf::Vector2f position = to_screen(fig, curve.x[0], curve.y[0], w, h);

    // Create the text object
    sf::Text text_obj;
    text_obj.setFont(font);
    text_obj.setString(sf::String::fromUtf8(curve.text_content.begin(), curve.text_content.end()));
    text_obj.setFillColor(curve.style.color);

    // Set character size based on style thickness or use default
    unsigned int char_size = curve.style.thickness > 0 ? static_cast<unsigned int>(curve.style.thickness * 6) : 12;
    text_obj.setCharacterSize(char_size);

    // Get text bounds to center it on the position point
    sf::FloatRect textRect = text_obj.getLocalBounds();

    // Position the text with a slight offset to avoid overlapping the exact point
    text_obj.setPosition(position.x - textRect.width / 2, position.y - textRect.height - 5);

    // Draw the text
    texture.draw(text_obj);
}

// Method for drawing an arrow head
void PlotGen::draw_arrow_head(const Figure &fig, const Figure::Curve &curve, double w, double h)
{
    if (curve.x.size() < 2 || curve.y.size() < 2)
        return; // Need at least a start and end point

    // Calculate screen coordinates for the arrow start and end
    sf::Vector2f start = to_screen(fig, curve.x[0], curve.y[0], w, h);
    sf::Vector2f end = to_screen(fig, curve.x[1], curve.y[1], w, h);

    // Calculate direction vector in screen coordinates
    sf::Vector2f direction = end - start;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    // If the arrow is too short, don't draw the head
    if (length < 1.0f)
        return;

    // Normalize the direction vector
    sf::Vector2f unitDirection = direction / length;

    // Calculate the perpendicular vector
    sf::Vector2f unitPerpendicular(-unitDirection.y, unitDirection.x);

    // Calculate the arrow head size in pixels
    // This value is adjustable according to visual preferences
    float head_length = curve.head_size * 0.5f; // Length in pixels
    float head_width = curve.head_size * 0.4f;  // Width in pixels

    // Calculate the arrow head points in screen coordinates
    sf::Vector2f tip = end;
    sf::Vector2f base = end - unitDirection * head_length;
    sf::Vector2f left = base + unitPerpendicular * (head_width / 2.0f);
    sf::Vector2f right = base - unitPerpendicular * (head_width / 2.0f);

    // Draw a filled triangle for the arrow head
    sf::ConvexShape arrowhead;
    arrowhead.setPointCount(3);
    arrowhead.setPoint(0, left);
    arrowhead.setPoint(1, tip);
    arrowhead.setPoint(2, right);
    arrowhead.setFillColor(curve.style.color);

    // Use an outline of the same color but slightly darker
    sf::Color outlineColor = curve.style.color;
    outlineColor.r = static_cast<sf::Uint8>(std::max(0, static_cast<int>(outlineColor.r * 0.8f)));
    outlineColor.g = static_cast<sf::Uint8>(std::max(0, static_cast<int>(outlineColor.g * 0.8f)));
    outlineColor.b = static_cast<sf::Uint8>(std::max(0, static_cast<int>(outlineColor.b * 0.8f)));

    arrowhead.setOutlineColor(outlineColor);
    arrowhead.setOutlineThickness(1.0f);

    // Draw the arrowhead
    texture.draw(arrowhead);
}

// Cubic Bezier curve with control points (x0,y0), (x1,y1), (x2,y2), (x3,y3)
void PlotGen::bezier(Figure &fig, double x0, double y0, double x1, double y1,
                     double x2, double y2, double x3, double y3,
                     const Style &style, int num_points)
{
    if (num_points < 2)
    {
        throw std::invalid_argument("Number of points must be at least 2");
    }

    // Generate points for the bezier curve
    std::vector<double> x(num_points), y(num_points);

    for (int i = 0; i < num_points; ++i)
    {
        double t = static_cast<double>(i) / (num_points - 1);

        // Cubic Bezier formula: B(t) = (1-t)³P₀ + 3(1-t)²tP₁ + 3(1-t)t²P₂ + t³P₃
        double mt = 1.0 - t;
        double mt2 = mt * mt;
        double mt3 = mt2 * mt;
        double t2 = t * t;
        double t3 = t2 * t;

        // Calculate point at parameter t
        x[i] = mt3 * x0 + 3 * mt2 * t * x1 + 3 * mt * t2 * x2 + t3 * x3;
        y[i] = mt3 * y0 + 3 * mt2 * t * y1 + 3 * mt * t2 * y2 + t3 * y3;
    }

    // Check if we need to adjust axis limits
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);

    if (using_default_limits)
    {
        // Find the extreme points to set appropriate axis limits
        double min_x = std::min({x0, x1, x2, x3});
        double max_x = std::max({x0, x1, x2, x3});
        double min_y = std::min({y0, y1, y2, y3});
        double max_y = std::max({y0, y1, y2, y3});

        // Add margin (10% of the data range)
        double x_margin = (max_x - min_x) * 0.1 + 1.0;
        double y_margin = (max_y - min_y) * 0.1 + 1.0;

        fig.xmin = min_x - x_margin;
        fig.xmax = max_x + x_margin;
        fig.ymin = min_y - y_margin;
        fig.ymax = max_y + y_margin;
    }

    // Store the bezier curve as a 2D curve
    fig.curves.push_back({x, y, style});
    fig.curve_types.push_back("2D");
}

// Bezier curve with control points
void PlotGen::bezier(Figure &fig, const std::vector<double> &x, const std::vector<double> &y,
                     const Style &style, int num_points)
{
    if (x.size() != y.size())
    {
        throw std::invalid_argument("x and y vectors must have the same size");
    }

    if (x.empty())
    {
        throw std::invalid_argument("Control points cannot be empty");
    }

    if (num_points < 2)
    {
        throw std::invalid_argument("Number of points must be at least 2");
    }

    // If we have exactly 4 control points, use the cubic Bezier function
    if (x.size() == 4)
    {
        bezier(fig, x[0], y[0], x[1], y[1], x[2], y[2], x[3], y[3], style, num_points);
        return;
    }

    // For other numbers of control points, implement de Casteljau's algorithm
    std::vector<double> result_x(num_points), result_y(num_points);

    for (int i = 0; i < num_points; ++i)
    {
        double t = static_cast<double>(i) / (num_points - 1);

        // Create copies of the control points
        std::vector<double> px = x;
        std::vector<double> py = y;

        // Apply de Casteljau's algorithm
        for (size_t j = 1; j < x.size(); ++j)
        {
            for (size_t k = 0; k < x.size() - j; ++k)
            {
                px[k] = (1 - t) * px[k] + t * px[k + 1];
                py[k] = (1 - t) * py[k] + t * py[k + 1];
            }
        }

        // The first point is now the point on the curve
        result_x[i] = px[0];
        result_y[i] = py[0];
    }

    // Check if we need to adjust axis limits
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);

    if (using_default_limits)
    {
        // Find the extreme points to set appropriate axis limits
        double min_x = *std::min_element(x.begin(), x.end());
        double max_x = *std::max_element(x.begin(), x.end());
        double min_y = *std::min_element(y.begin(), y.end());
        double max_y = *std::max_element(y.begin(), y.end());

        // Add margin (10% of the data range)
        double x_margin = (max_x - min_x) * 0.1 + 1.0;
        double y_margin = (max_y - min_y) * 0.1 + 1.0;

        fig.xmin = min_x - x_margin;
        fig.xmax = max_x + x_margin;
        fig.ymin = min_y - y_margin;
        fig.ymax = max_y + y_margin;
    }

    // Store the bezier curve as a 2D curve
    fig.curves.push_back({result_x, result_y, style});
    fig.curve_types.push_back("2D");
}

// Natural cubic spline through points
void PlotGen::spline(Figure &fig, const std::vector<double> &x, const std::vector<double> &y,
                     const Style &style, int num_points)
{
    if (x.size() != y.size())
    {
        throw std::invalid_argument("x and y vectors must have the same size");
    }

    if (x.size() < 2)
    {
        throw std::invalid_argument("At least two points are needed for a spline");
    }

    if (num_points < 2)
    {
        throw std::invalid_argument("Number of points must be at least 2");
    }

    size_t n = x.size();

    // Sort points by x-coordinate if not already sorted
    std::vector<size_t> indices(n);
    for (size_t i = 0; i < n; ++i)
        indices[i] = i;

    std::sort(indices.begin(), indices.end(), [&x](size_t i1, size_t i2)
              { return x[i1] < x[i2]; });

    std::vector<double> sorted_x(n), sorted_y(n);
    for (size_t i = 0; i < n; ++i)
    {
        sorted_x[i] = x[indices[i]];
        sorted_y[i] = y[indices[i]];
    }

    // Calculate the second derivatives at each point (natural spline conditions)
    std::vector<double> h(n - 1);
    for (size_t i = 0; i < n - 1; ++i)
    {
        h[i] = sorted_x[i + 1] - sorted_x[i];
        if (h[i] <= 0)
        {
            throw std::invalid_argument("Points must have strictly increasing x values");
        }
    }

    // Set up the tridiagonal system
    std::vector<double> alpha(n - 2, 0.0);
    for (size_t i = 0; i < n - 2; ++i)
    {
        alpha[i] = 3.0 * ((sorted_y[i + 2] - sorted_y[i + 1]) / h[i + 1] -
                          (sorted_y[i + 1] - sorted_y[i]) / h[i]);
    }

    // Solve the tridiagonal system using Thomas algorithm
    std::vector<double> l(n - 2, 0.0);
    std::vector<double> mu(n - 2, 0.0);
    std::vector<double> z(n - 2, 0.0);
    std::vector<double> c(n, 0.0); // Second derivatives

    // Forward sweep
    l[0] = 2.0 * (h[0] + h[1]);
    mu[0] = 0.5;
    z[0] = alpha[0] / l[0];

    for (size_t i = 1; i < n - 2; ++i)
    {
        l[i] = 2.0 * (h[i] + h[i + 1]) - h[i] * mu[i - 1];
        mu[i] = h[i + 1] / l[i];
        z[i] = (alpha[i] - h[i] * z[i - 1]) / l[i];
    }

    // Backward sweep
    for (int j = n - 3; j >= 0; --j)
    {
        c[j + 1] = z[j] - mu[j] * c[j + 2];
    }

    // Calculate coefficients for each segment
    std::vector<double> a(n - 1), b(n - 1), d(n - 1);

    for (size_t i = 0; i < n - 1; ++i)
    {
        a[i] = sorted_y[i];
        b[i] = (sorted_y[i + 1] - sorted_y[i]) / h[i] - h[i] * (c[i + 1] + 2.0 * c[i]) / 3.0;
        d[i] = (c[i + 1] - c[i]) / (3.0 * h[i]);
    }

    // Generate points for the spline curve
    std::vector<double> result_x, result_y;
    result_x.reserve(num_points);
    result_y.reserve(num_points);

    // Calculate number of points per segment
    int points_per_segment = num_points / (n - 1);
    int extra_points = num_points % (n - 1);

    for (size_t i = 0; i < n - 1; ++i)
    {
        int seg_points = points_per_segment + (i < static_cast<size_t>(extra_points) ? 1 : 0);

        for (int j = 0; j < seg_points; ++j)
        {
            // Don't duplicate points between segments
            if (i > 0 && j == 0)
                continue;

            double t = static_cast<double>(j) / seg_points;
            double dx = sorted_x[i + 1] - sorted_x[i];
            double x_val = sorted_x[i] + t * dx;

            double s = x_val - sorted_x[i];
            double y_val = a[i] + b[i] * s + c[i] * s * s + d[i] * s * s * s;

            result_x.push_back(x_val);
            result_y.push_back(y_val);
        }
    }

    // Add the last point
    result_x.push_back(sorted_x[n - 1]);
    result_y.push_back(sorted_y[n - 1]);

    // Ensure we have exactly num_points points
    while (result_x.size() < static_cast<size_t>(num_points))
    {
        // Add duplicate of last point
        result_x.push_back(result_x.back());
        result_y.push_back(result_y.back());
    }

    while (result_x.size() > static_cast<size_t>(num_points))
    {
        // Remove points from the end
        result_x.pop_back();
        result_y.pop_back();
    }

    // Check if we need to adjust axis limits
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);

    if (using_default_limits)
    {
        // Find the extreme points in the resulting curve
        double min_x = *std::min_element(result_x.begin(), result_x.end());
        double max_x = *std::max_element(result_x.begin(), result_x.end());
        double min_y = *std::min_element(result_y.begin(), result_y.end());
        double max_y = *std::max_element(result_y.begin(), result_y.end());

        // Add margin (10% of the data range)
        double x_margin = (max_x - min_x) * 0.1 + 1.0;
        double y_margin = (max_y - min_y) * 0.1 + 1.0;

        fig.xmin = min_x - x_margin;
        fig.xmax = max_x + x_margin;
        fig.ymin = min_y - y_margin;
        fig.ymax = max_y + y_margin;
    }

    // Store the spline curve as a 2D curve
    fig.curves.push_back({result_x, result_y, style});
    fig.curve_types.push_back("2D");
}

// Cardinal spline through points with tension parameter
void PlotGen::cardinal_spline(Figure &fig, const std::vector<double> &x, const std::vector<double> &y,
                              double tension, const Style &style, int num_points)
{
    if (x.size() != y.size())
    {
        throw std::invalid_argument("x and y vectors must have the same size");
    }

    if (x.size() < 2)
    {
        throw std::invalid_argument("At least two points are needed for a spline");
    }

    if (num_points < 2)
    {
        throw std::invalid_argument("Number of points must be at least 2");
    }

    // Constrain tension parameter between 0 and 1
    tension = std::max(0.0, std::min(1.0, tension));

    // For Cardinal splines, we use a tension factor c
    // c = 0 gives a Catmull-Rom spline (tight)
    // c = 1 gives a linear interpolation (loose)
    double c = 1.0 - tension;

    size_t n = x.size();

    // Generate points for the cardinal spline
    std::vector<double> result_x, result_y;
    result_x.reserve(num_points);
    result_y.reserve(num_points);

    // Handle the case of just 2 points - straight line
    if (n == 2)
    {
        for (int i = 0; i < num_points; ++i)
        {
            double t = static_cast<double>(i) / (num_points - 1);
            result_x.push_back(x[0] + t * (x[1] - x[0]));
            result_y.push_back(y[0] + t * (y[1] - y[0]));
        }
    }
    else
    {
        // For each segment between points
        int points_per_segment = num_points / (n - 1);
        int extra_points = num_points % (n - 1);

        for (size_t i = 0; i < n - 1; ++i)
        {
            // Get the four points needed for this segment
            double x0, y0, x1, y1, x2, y2, x3, y3;

            // First point (can be virtual for the first segment)
            if (i == 0)
            {
                // Create a virtual point before the first point
                x0 = x[0] - (x[1] - x[0]);
                y0 = y[0] - (y[1] - y[0]);
            }
            else
            {
                x0 = x[i - 1];
                y0 = y[i - 1];
            }

            // Middle two points (actual segment)
            x1 = x[i];
            y1 = y[i];
            x2 = x[i + 1];
            y2 = y[i + 1];

            // Last point (can be virtual for the last segment)
            if (i == n - 2)
            {
                // Create a virtual point after the last point
                x3 = x[n - 1] + (x[n - 1] - x[n - 2]);
                y3 = y[n - 1] + (y[n - 1] - y[n - 2]);
            }
            else
            {
                x3 = x[i + 2];
                y3 = y[i + 2];
            }

            // Calculate tangents at points p1 and p2
            double m1_x = c * (x2 - x0) / 2;
            double m1_y = c * (y2 - y0) / 2;
            double m2_x = c * (x3 - x1) / 2;
            double m2_y = c * (y3 - y1) / 2;

            int seg_points = points_per_segment + (i < static_cast<size_t>(extra_points) ? 1 : 0);

            // Generate points for this segment
            for (int j = 0; j < seg_points; ++j)
            {
                // Skip duplicated points between segments
                if (i > 0 && j == 0)
                    continue;

                double t = static_cast<double>(j) / seg_points;
                double t2 = t * t;
                double t3 = t2 * t;

                // Hermite basis functions
                double h1 = 2 * t3 - 3 * t2 + 1;
                double h2 = -2 * t3 + 3 * t2;
                double h3 = t3 - 2 * t2 + t;
                double h4 = t3 - t2;

                // Calculate point
                double x_val = h1 * x1 + h2 * x2 + h3 * m1_x + h4 * m2_x;
                double y_val = h1 * y1 + h2 * y2 + h3 * m1_y + h4 * m2_y;

                result_x.push_back(x_val);
                result_y.push_back(y_val);
            }
        }

        // Add the last point
        result_x.push_back(x[n - 1]);
        result_y.push_back(y[n - 1]);
    }

    // Ensure we have exactly num_points points
    while (result_x.size() < static_cast<size_t>(num_points))
    {
        // Add duplicate of last point
        result_x.push_back(result_x.back());
        result_y.push_back(result_y.back());
    }

    while (result_x.size() > static_cast<size_t>(num_points))
    {
        // Remove points from the end
        result_x.pop_back();
        result_y.pop_back();
    }

    // Check if we need to adjust axis limits
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);

    if (using_default_limits)
    {
        // Find the extreme points in the resulting curve
        double min_x = *std::min_element(result_x.begin(), result_x.end());
        double max_x = *std::max_element(result_x.begin(), result_x.end());
        double min_y = *std::min_element(result_y.begin(), result_y.end());
        double max_y = *std::max_element(result_y.begin(), result_y.end());

        // Add margin (10% of the data range)
        double x_margin = (max_x - min_x) * 0.1 + 1.0;
        double y_margin = (max_y - min_y) * 0.1 + 1.0;

        fig.xmin = min_x - x_margin;
        fig.xmax = max_x + x_margin;
        fig.ymin = min_y - y_margin;
        fig.ymax = max_y + y_margin;
    }

    // Store the cardinal spline curve as a 2D curve
    fig.curves.push_back({result_x, result_y, style});
    fig.curve_types.push_back("2D");
}

// Function to convert sf::Color to SVG color string
std::string PlotGen::color_to_svg(const sf::Color &color)
{
    char buffer[20];
    std::snprintf(buffer, sizeof(buffer), "#%02x%02x%02x", color.r, color.g, color.b);
    return std::string(buffer);
}

// Function to convert line style to SVG dash array
std::string PlotGen::line_style_to_svg(const std::string &line_style, float thickness)
{
    if (line_style == "dashed")
    {
        return "stroke-dasharray=\"" + std::to_string(5 * thickness) + "," + std::to_string(3 * thickness) + "\"";
    }
    else if (line_style == "dotted")
    {
        return "stroke-dasharray=\"" + std::to_string(1 * thickness) + "," + std::to_string(2 * thickness) + "\"";
    }
    return ""; // solid line
}

// Export curve to SVG
void PlotGen::export_svg_curve(const Figure &fig, const Figure::Curve &curve, std::ofstream &svg_file,
                               double x_offset, double y_offset, double width, double height)
{
    // Si c'est une courbe "none" (pas de ligne), ne rien faire ici, les symboles seront ajoutés plus bas
    if (curve.style.line_style != "none" && curve.style.line_style != "points")
    {
        // Path for the line
        svg_file << "<path d=\"M";

        for (size_t i = 0; i < curve.x.size(); ++i)
        {
            double sx = x_offset + (curve.x[i] - fig.xmin) / (fig.xmax - fig.xmin) * width;
            double sy = y_offset + height - (curve.y[i] - fig.ymin) / (fig.ymax - fig.ymin) * height;

            if (i == 0)
            {
                svg_file << sx << " " << sy;
            }
            else
            {
                svg_file << " L " << sx << " " << sy;
            }
        }

        // Close path and add styling
        svg_file << "\" fill=\"none\" stroke=\""
                 << color_to_svg(curve.style.color)
                 << "\" stroke-width=\"" << curve.style.thickness << "\" ";

        // Ajout correct du style de ligne (solide ou pointillé)
        if (curve.style.line_style == "dashed")
        {
            svg_file << "stroke-dasharray=\"" << (5 * curve.style.thickness) << ","
                     << (3 * curve.style.thickness) << "\"";
        }
        else if (curve.style.line_style == "dotted")
        {
            svg_file << "stroke-dasharray=\"" << (1 * curve.style.thickness) << ","
                     << (2 * curve.style.thickness) << "\"";
        }
        svg_file << " />\n";
    }

    // Add symbols if needed
    if (curve.style.symbol_type != "none" && curve.style.symbol_size > 0)
    {
        for (size_t i = 0; i < curve.x.size(); ++i)
        {
            double sx = x_offset + (curve.x[i] - fig.xmin) / (fig.xmax - fig.xmin) * width;
            double sy = y_offset + height - (curve.y[i] - fig.ymin) / (fig.ymax - fig.ymin) * height;

            // Draw different symbols based on symbol_type
            if (curve.style.symbol_type == "circle")
            {
                svg_file << "<circle cx=\"" << sx << "\" cy=\"" << sy
                         << "\" r=\"" << curve.style.symbol_size / 2
                         << "\" fill=\"" << color_to_svg(curve.style.color)
                         << "\" stroke=\"black\" stroke-width=\"1\" />\n";
            }
            else if (curve.style.symbol_type == "square")
            {
                double halfSize = curve.style.symbol_size / 2;
                svg_file << "<rect x=\"" << (sx - halfSize) << "\" y=\"" << (sy - halfSize)
                         << "\" width=\"" << curve.style.symbol_size << "\" height=\"" << curve.style.symbol_size
                         << "\" fill=\"" << color_to_svg(curve.style.color)
                         << "\" stroke=\"black\" stroke-width=\"1\" />\n";
            }
            else if (curve.style.symbol_type == "triangle")
            {
                // Créer un triangle équilatéral
                double halfSize = curve.style.symbol_size / 2;
                double height = curve.style.symbol_size * 0.866; // Hauteur d'un triangle équilatéral = côté * √3/2

                // Calculer les coordonnées des sommets
                std::string points = std::to_string(sx) + "," + std::to_string(sy - halfSize) + " " +
                                     std::to_string(sx - halfSize) + "," + std::to_string(sy + height / 2) + " " +
                                     std::to_string(sx + halfSize) + "," + std::to_string(sy + height / 2);

                svg_file << "<polygon points=\"" << points
                         << "\" fill=\"" << color_to_svg(curve.style.color)
                         << "\" stroke=\"black\" stroke-width=\"1\" />\n";
            }
            else if (curve.style.symbol_type == "diamond")
            {
                // Créer un losange (carré tourné à 45°)
                double halfSize = curve.style.symbol_size / 2;

                // Calculer les coordonnées des sommets
                std::string points = std::to_string(sx) + "," + std::to_string(sy - halfSize) + " " +
                                     std::to_string(sx + halfSize) + "," + std::to_string(sy) + " " +
                                     std::to_string(sx) + "," + std::to_string(sy + halfSize) + " " +
                                     std::to_string(sx - halfSize) + "," + std::to_string(sy);

                svg_file << "<polygon points=\"" << points
                         << "\" fill=\"" << color_to_svg(curve.style.color)
                         << "\" stroke=\"black\" stroke-width=\"1\" />\n";
            }
            else if (curve.style.symbol_type == "star")
            {
                // Créer une étoile à 5 branches
                const int numPoints = 5;
                const double innerRadius = curve.style.symbol_size / 4;
                const double outerRadius = curve.style.symbol_size / 2;

                std::string points;
                for (int j = 0; j < numPoints * 2; j++)
                {
                    double radius = (j % 2 == 0) ? outerRadius : innerRadius;
                    double angle = j * M_PI / numPoints;
                    double x = sx + radius * std::sin(angle);
                    double y = sy - radius * std::cos(angle);

                    if (j > 0)
                        points += " ";
                    points += std::to_string(x) + "," + std::to_string(y);
                }

                svg_file << "<polygon points=\"" << points
                         << "\" fill=\"" << color_to_svg(curve.style.color)
                         << "\" stroke=\"black\" stroke-width=\"1\" />\n";
            }
        }
    }
}

// Export grid to SVG
void PlotGen::export_svg_grid(const Figure &fig, std::ofstream &svg_file,
                              double x_offset, double y_offset, double width, double height)
{
    // Draw major grid lines
    if (fig.show_major_grid)
    {
        // Number of major grid lines
        const int numTicksX = 5;
        const int numTicksY = 5;

        // Draw vertical lines
        for (int i = 0; i <= numTicksX; ++i)
        {
            double x = fig.xmin + (fig.xmax - fig.xmin) * i / numTicksX;
            double sx = x_offset + (x - fig.xmin) / (fig.xmax - fig.xmin) * width;

            svg_file << "<line x1=\"" << sx << "\" y1=\"" << y_offset
                     << "\" x2=\"" << sx << "\" y2=\"" << (y_offset + height)
                     << "\" stroke=\"" << color_to_svg(fig.major_grid_color)
                     << "\" stroke-width=\"1\" />\n";
        }

        // Draw horizontal lines
        for (int i = 0; i <= numTicksY; ++i)
        {
            double y = fig.ymin + (fig.ymax - fig.ymin) * i / numTicksY;
            double sy = y_offset + height - (y - fig.ymin) / (fig.ymax - fig.ymin) * height;

            svg_file << "<line x1=\"" << x_offset << "\" y1=\"" << sy
                     << "\" x2=\"" << (x_offset + width) << "\" y2=\"" << sy
                     << "\" stroke=\"" << color_to_svg(fig.major_grid_color)
                     << "\" stroke-width=\"1\" />\n";
        }
    }

    // Draw minor grid lines
    if (fig.show_minor_grid)
    {
        // Number of major divisions
        const int numMajorX = 5;
        const int numMajorY = 5;

        // Number of minor subdivisions between each major line
        const int numMinorSubdivisions = 4;

        // Draw minor vertical lines
        for (int i = 0; i < numMajorX; ++i)
        {
            double x_start = fig.xmin + (fig.xmax - fig.xmin) * i / numMajorX;
            double x_step = (fig.xmax - fig.xmin) / (numMajorX * numMinorSubdivisions);

            for (int j = 1; j < numMinorSubdivisions; ++j)
            {
                double x = x_start + j * x_step;
                double sx = x_offset + (x - fig.xmin) / (fig.xmax - fig.xmin) * width;

                svg_file << "<line x1=\"" << sx << "\" y1=\"" << y_offset
                         << "\" x2=\"" << sx << "\" y2=\"" << (y_offset + height)
                         << "\" stroke=\"" << color_to_svg(fig.minor_grid_color)
                         << "\" stroke-width=\"1\" />\n";
            }
        }

        // Draw minor horizontal lines
        for (int i = 0; i < numMajorY; ++i)
        {
            double y_start = fig.ymin + (fig.ymax - fig.ymin) * i / numMajorY;
            double y_step = (fig.ymax - fig.ymin) / (numMajorY * numMinorSubdivisions);

            for (int j = 1; j < numMinorSubdivisions; ++j)
            {
                double y = y_start + j * y_step;
                double sy = y_offset + height - (y - fig.ymin) / (fig.ymax - fig.ymin) * height;

                svg_file << "<line x1=\"" << x_offset << "\" y1=\"" << sy
                         << "\" x2=\"" << (x_offset + width) << "\" y2=\"" << sy
                         << "\" stroke=\"" << color_to_svg(fig.minor_grid_color)
                         << "\" stroke-width=\"1\" />\n";
            }
        }
    }
}

// Export une grille polaire en SVG
void PlotGen::export_svg_polar_grid(const Figure &fig, std::ofstream &svg_file,
                                    double x_offset, double y_offset, double width, double height)
{
    // Centre de la grille polaire
    double center_x = x_offset + width / 2;
    double center_y = y_offset + height / 2;

    // Rayon maximum pour les cercles (en pixels)
    double max_radius = std::min(width, height) / 2 - 20;

    // Rayon maximum des données (en unités)
    double max_r = std::max(std::abs(fig.xmax), std::abs(fig.ymax));

    // Nombre de cercles concentriques pour la grille principale
    const int numCircles = 5;

    // Nombre de rayons pour la grille principale (un rayon tous les 30 degrés)
    const int numRays = 12;

    // Dessiner les cercles concentriques (grille principale)
    if (fig.show_major_grid)
    {
        // Cercles concentriques
        for (int i = 1; i <= numCircles; ++i)
        {
            double radius = max_radius * i / numCircles;
            double r_value = max_r * i / numCircles;

            svg_file << "<circle cx=\"" << center_x << "\" cy=\"" << center_y
                     << "\" r=\"" << radius << "\" fill=\"none\" stroke=\""
                     << color_to_svg(fig.major_grid_color) << "\" stroke-width=\"1\"/>\n";

            // Ajouter des étiquettes de rayon
            double angle = M_PI / 4; // 45 degrés, position du label
            double label_x = center_x + radius * std::cos(angle);
            double label_y = center_y - radius * std::sin(angle);

            // Formatage précis pour les valeurs de rayon (avec 1 décimale)
            char r_buffer[10];
            std::snprintf(r_buffer, sizeof(r_buffer), "%.1f", r_value);

            svg_file << "<text x=\"" << label_x << "\" y=\"" << label_y
                     << "\" text-anchor=\"middle\" dominant-baseline=\"middle\" "
                     << "font-family=\"Arial\" font-size=\"10\" fill=\"black\">"
                     << r_buffer << "</text>\n";
        }

        // Rayons depuis le centre
        for (int i = 0; i < numRays; ++i)
        {
            double angle = 2 * M_PI * i / numRays;
            double end_x = center_x + max_radius * std::cos(angle);
            double end_y = center_y - max_radius * std::sin(angle);

            svg_file << "<line x1=\"" << center_x << "\" y1=\"" << center_y
                     << "\" x2=\"" << end_x << "\" y2=\"" << end_y
                     << "\" stroke=\"" << color_to_svg(fig.major_grid_color)
                     << "\" stroke-width=\"1\"/>\n";

            // Ajouter des étiquettes d'angle (en degrés)
            double degrees = angle * 180 / M_PI;
            if (degrees >= 360)
                degrees -= 360;

            // Formatage précis pour les angles (avec 1 décimale)
            char angle_buffer[15];
            std::snprintf(angle_buffer, sizeof(angle_buffer), "%.1f°", degrees);

            // Position du label, légèrement au-delà de l'extrémité du rayon
            double label_x = center_x + (max_radius + 15) * std::cos(angle);
            double label_y = center_y - (max_radius + 15) * std::sin(angle);

            svg_file << "<text x=\"" << label_x << "\" y=\"" << label_y
                     << "\" text-anchor=\"middle\" dominant-baseline=\"middle\" "
                     << "font-family=\"Arial\" font-size=\"10\" fill=\"black\">"
                     << angle_buffer << "</text>\n";
        }
    }

    // Dessiner la grille mineure
    if (fig.show_minor_grid)
    {
        // Cercles concentriques mineurs entre les cercles principaux
        const int numMinorCircles = 4;
        for (int i = 0; i < numCircles; ++i)
        {
            double radiusStart = max_radius * i / numCircles;
            double radiusStep = max_radius / numCircles / numMinorCircles;

            for (int j = 1; j < numMinorCircles; ++j)
            {
                double radius = radiusStart + j * radiusStep;

                svg_file << "<circle cx=\"" << center_x << "\" cy=\"" << center_y
                         << "\" r=\"" << radius << "\" fill=\"none\" stroke=\""
                         << color_to_svg(fig.minor_grid_color) << "\" stroke-width=\"1\"/>\n";
            }
        }

        // Rayons mineurs entre les rayons principaux
        const int numMinorRays = numRays * 2; // Un rayon tous les 15 degrés
        for (int i = 0; i < numMinorRays; ++i)
        {
            // Ignorer les rayons qui sont déjà dans la grille principale
            if (i % 2 == 0)
                continue;

            double angle = 2 * M_PI * i / numMinorRays;
            double end_x = center_x + max_radius * std::cos(angle);
            double end_y = center_y - max_radius * std::sin(angle);

            svg_file << "<line x1=\"" << center_x << "\" y1=\"" << center_y
                     << "\" x2=\"" << end_x << "\" y2=\"" << end_y
                     << "\" stroke=\"" << color_to_svg(fig.minor_grid_color)
                     << "\" stroke-width=\"1\"/>\n";
        }
    }
}

// Export histogram to SVG
void PlotGen::export_svg_histogram(const Figure &fig, const Figure::Curve &curve, std::ofstream &svg_file,
                                   double x_offset, double y_offset, double width, double height)
{
    if (curve.x.empty() || curve.y.empty())
        return;

    // Calculate bin width based on data
    double bin_width = 0;
    if (curve.x.size() > 1)
    {
        bin_width = curve.x[1] - curve.x[0];
    }
    else
    {
        bin_width = (fig.xmax - fig.xmin) / 20.0;
    }

    // Calculate bar width in SVG coordinates
    double bar_width_svg = bin_width * width / (fig.xmax - fig.xmin);

    // Apply width ratio
    bar_width_svg *= curve.bar_width_ratio;

    // Find maximum Y value if needed
    // double max_y = *std::max_element(curve.y.begin(), curve.y.end());

    // Draw each bar
    for (size_t i = 0; i < curve.x.size(); ++i)
    {
        // Convert data coordinates to SVG coordinates
        double sx = x_offset + (curve.x[i] - fig.xmin) / (fig.xmax - fig.xmin) * width;

        // Calculate proper y position and height, ensuring we start from y=0
        double bar_height_data = curve.y[i];
        double bar_height_svg = (bar_height_data / (fig.ymax - fig.ymin)) * height;

        // Y position is bottom of the bar (remember SVG y-axis grows downward)
        double sy = y_offset + height - (bar_height_data - fig.ymin) / (fig.ymax - fig.ymin) * height;

        // Draw rectangle
        svg_file << "<rect x=\"" << (sx - bar_width_svg / 2) << "\" y=\"" << sy
                 << "\" width=\"" << bar_width_svg << "\" height=\"" << bar_height_svg
                 << "\" fill=\"" << color_to_svg(curve.style.color)
                 << "\" stroke=\"black\" stroke-width=\"1\" />\n";
    }
}

#ifdef HAVE_GTK_WEBKIT
// Implementation of the HTMLViewer class
HTMLViewer::HTMLViewer() : window_handle(nullptr), initialized(false), svg_width(800), svg_height(600), current_svg_content(""), temp_svg_file("")
{
}

HTMLViewer::~HTMLViewer()
{
    close();
}

bool HTMLViewer::initialize()
{
    if (initialized)
        return true;

    // Initialize GTK if necessary
    if (!gtk_init_check(nullptr, nullptr))
    {
        std::cerr << "Error: Unable to initialize GTK" << std::endl;
        return false;
    }

    initialized = true;
    return true;
}

bool HTMLViewer::loadAndDisplayHTML(const std::string &html_file)
{
    if (!initialized && !initialize())
        return false;

    // Create a new GTK window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "PlotGenC++ - SVG Viewer");

    // Connect destroy signal to quit GTK main loop
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    // Connect key-press-event signal to handle Escape key
    g_signal_connect(window, "key-press-event", G_CALLBACK(+[](GtkWidget *widget, GdkEventKey *event, gpointer data) -> gboolean
                                                           {
                                                               if (event->keyval == GDK_KEY_Escape)
                                                               {
                                                                   gtk_main_quit();
                                                                   return TRUE;
                                                               }
                                                               return FALSE;
                                                           }),
                     nullptr);

    // Create a structure to store data needed for callbacks
    struct ViewerData
    {
        std::string svg_content;
        std::string current_file_path;
        std::string temp_svg_file; // Path to the temporary SVG file to be deleted
    };

    ViewerData *data = new ViewerData();
    data->svg_content = this->current_svg_content;
    data->current_file_path = html_file;
    data->temp_svg_file = this->temp_svg_file;

    // Create a vertical box to contain the menu and webview
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create the menu bar
    GtkWidget *menu_bar = gtk_menu_bar_new();

    // File Menu
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);

    // Save SVG Option
    GtkWidget *save_item = gtk_menu_item_new_with_label("Save SVG");
    g_signal_connect(save_item, "activate", G_CALLBACK(+[](GtkWidget *widget, gpointer user_data) -> void
                                                       {
                                                           ViewerData *data = static_cast<ViewerData *>(user_data);

                                                           GtkWidget *dialog = gtk_file_chooser_dialog_new("Save SVG",
                                                                                                           nullptr,
                                                                                                           GTK_FILE_CHOOSER_ACTION_SAVE,
                                                                                                           "_Cancel", GTK_RESPONSE_CANCEL,
                                                                                                           "_Save", GTK_RESPONSE_ACCEPT,
                                                                                                           nullptr);

                                                           // Suggest a default filename with .svg extension
                                                           gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "chart.svg");

                                                           if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
                                                           {
                                                               char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

                                                               // Extract SVG content from HTML
                                                               size_t start = data->svg_content.find("<svg");
                                                               size_t end = data->svg_content.find("</svg>") + 6; // +6 to include "</svg>"

                                                               if (start != std::string::npos && end != std::string::npos)
                                                               {
                                                                   std::string svg_content = data->svg_content.substr(start, end - start);

                                                                   // Save SVG content to the selected file
                                                                   std::ofstream file(filename);
                                                                   if (file.is_open())
                                                                   {
                                                                       file << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
                                                                            << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
                                                                            << svg_content;
                                                                       file.close();

                                                                       // Display success message
                                                                       GtkWidget *message = gtk_message_dialog_new(nullptr,
                                                                                                                   GTK_DIALOG_MODAL,
                                                                                                                   GTK_MESSAGE_INFO,
                                                                                                                   GTK_BUTTONS_OK,
                                                                                                                   "The SVG file has been saved successfully.");
                                                                       gtk_dialog_run(GTK_DIALOG(message));
                                                                       gtk_widget_destroy(message);
                                                                   }
                                                               }
                                                               g_free(filename);
                                                           }

                                                           gtk_widget_destroy(dialog);
                                                       }),
                     data);

    // Exit Option
    GtkWidget *quit_item = gtk_menu_item_new_with_label("Exit");
    g_signal_connect(quit_item, "activate", G_CALLBACK(+[](GtkWidget *widget, gpointer user_data) -> void
                                                       {
                                                           gtk_main_quit();
                                                       }),
                     nullptr);

    // Add options to the File menu
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_item);

    // Help Menu
    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *help_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);

    // About Option
    GtkWidget *about_item = gtk_menu_item_new_with_label("About");
    g_signal_connect(about_item, "activate", G_CALLBACK(+[](GtkWidget *widget, gpointer user_data) -> void
                                                        {
                                                            GtkWidget *dialog = gtk_about_dialog_new();
                                                            gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "PlotGenC++");
                                                            gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "1.0");
                                                            gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "© 2025");
                                                            gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog),
                                                                                          "PlotGenC++ is a plotting library for C++.\n"
                                                                                          "It allows creating high-quality scientific plots with a simple and intuitive API.");
                                                            gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "https://github.com/skhelladi/plotgencpp");

                                                            gtk_dialog_run(GTK_DIALOG(dialog));
                                                            gtk_widget_destroy(dialog);
                                                        }),
                     nullptr);

    // Add option to the Help menu
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_item);

    // Add menus to the menu bar
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_item);

    // Add the menu bar to the vertical box
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

    // Create a WebKit widget to display HTML/SVG content
    GtkWidget *webview = webkit_web_view_new();
    gtk_box_pack_start(GTK_BOX(vbox), webview, TRUE, TRUE, 0);

    // Connect destroy signal to free data and delete temporary files
    g_signal_connect(window, "destroy", G_CALLBACK(+[](GtkWidget *widget, gpointer user_data) -> void
                                                   {
                                                       ViewerData *data = static_cast<ViewerData *>(user_data);

                                                       // Delete the temporary SVG file if it exists
                                                       if (!data->temp_svg_file.empty())
                                                       {
                                                           std::remove(data->temp_svg_file.c_str());
                                                           std::cout << "Temporary file deleted: " << data->temp_svg_file << std::endl;
                                                       }

                                                       // Also delete the temporary HTML file
                                                       if (!data->current_file_path.empty() && data->current_file_path.find("/tmp/plotgencpp_") == 0)
                                                       {
                                                           std::remove(data->current_file_path.c_str());
                                                           std::cout << "Temporary HTML file deleted: " << data->current_file_path << std::endl;
                                                       }

                                                       delete data;
                                                   }),
                     data);

    // Connect realize signal to maximize window after it's fully created
    g_signal_connect(window, "realize", G_CALLBACK(+[](GtkWidget *widget, gpointer data) -> void
                                                   {
                                                       // Maximize the window once it's fully realized
                                                       gtk_window_maximize(GTK_WINDOW(widget));

                                                       // Process pending events to ensure maximize takes effect
                                                       while (gtk_events_pending())
                                                           gtk_main_iteration_do(FALSE);

                                                       // Add a short delay and a second call to maximize to ensure maximization is applied
                                                       g_timeout_add(100, [](gpointer user_data) -> gboolean
                                                                     {
                                                                         GtkWidget *window = GTK_WIDGET(user_data);
                                                                         gtk_window_maximize(GTK_WINDOW(window));
                                                                         return FALSE; // Function executed only once
                                                                     },
                                                                     widget);
                                                   }),
                     nullptr);

    // Load the HTML file
    std::string uri = "file://" + html_file;
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), uri.c_str());

    // Store the window pointer
    window_handle = window;

    // Display the window and start the GTK main loop
    gtk_widget_show_all(window);

    // We no longer call gtk_window_maximize here, it will be done in the "realize" signal

    gtk_main();

    return true;
}

bool HTMLViewer::loadAndDisplaySVG(const std::string &svg_content, const std::string &temp_svg_file)
{
    // Store the SVG content and temporary file path
    current_svg_content = svg_content;
    this->temp_svg_file = temp_svg_file;

    // Create a temporary HTML file containing the SVG
    std::string html_file = createTempHTMLWithSVG(svg_content);
    if (html_file.empty())
        return false;

    return loadAndDisplayHTML(html_file);
}

void HTMLViewer::close()
{
    if (window_handle)
    {
        gtk_widget_destroy(GTK_WIDGET(window_handle));
        window_handle = nullptr;
    }
}

std::string HTMLViewer::createTempHTMLWithSVG(const std::string &svg_content)
{
    // Generate a unique temporary filename
    std::string temp_filename = "/tmp/plotgencpp_" + std::to_string(time(nullptr)) + ".html";

    // Create the HTML file
    std::ofstream html_file(temp_filename);
    if (!html_file.is_open())
    {
        std::cerr << "Error: Unable to create temporary HTML file" << std::endl;
        return "";
    }

    // Parse SVG width and height from content
    size_t width_pos = svg_content.find("width=\"");
    size_t height_pos = svg_content.find("height=\"");

    if (width_pos != std::string::npos && height_pos != std::string::npos)
    {
        width_pos += 7;  // Length of 'width="'
        height_pos += 8; // Length of 'height="'

        size_t width_end = svg_content.find("\"", width_pos);
        size_t height_end = svg_content.find("\"", height_pos);

        if (width_end != std::string::npos && height_end != std::string::npos)
        {
            std::string width_str = svg_content.substr(width_pos, width_end - width_pos);
            std::string height_str = svg_content.substr(height_pos, height_end - height_pos);

            try
            {
                svg_width = std::stoi(width_str);
                svg_height = std::stoi(height_str);
                std::cout << "Extracted SVG dimensions: " << svg_width << "x" << svg_height << std::endl;
            }
            catch (...)
            {
                std::cerr << "Warning: Failed to parse SVG dimensions, using defaults" << std::endl;
                // Use default values if parsing fails
            }
        }
    }

    // Write HTML content with the SVG embedded
    html_file << "<!DOCTYPE html>\n"
              << "<html>\n"
              << "<head>\n"
              << "    <meta charset=\"UTF-8\">\n"
              << "    <title>PlotGenC++ - SVG Visualization</title>\n"
              << "    <style>\n"
              << "        body { margin: 0; padding: 0; overflow: hidden; background-color: #f0f0f0; }\n"
              << "        #svg-container { width: 100vw; height: 100vh; display: flex; justify-content: center; align-items: center; }\n"
              << "        #controls { position: fixed; bottom: 10px; left: 10px; background: rgba(255,255,255,0.8); padding: 10px; border-radius: 5px; }\n"
              << "        svg { max-width: 95%; max-height: 95%; background-color: white; box-shadow: 0 0 10px rgba(0,0,0,0.2); }\n"
              << "        button { padding: 5px 10px; cursor: pointer; }\n"
              << "    </style>\n"
              << "</head>\n"
              << "<body>\n"
              << "    <div id=\"svg-container\">\n"
              << svg_content << "\n"
              << "    </div>\n"
              << "</body>\n"
              << "</html>";

    html_file.close();
    return temp_filename;
}

// Implementation of the PlotGen::show_with_viewer method
void PlotGen::show_with_viewer()
{
    // Generate a unique temporary SVG filename
    std::string svg_filename = "plot_" + std::to_string(time(nullptr)) + ".svg";

    // Save as SVG
    save_svg(svg_filename);

    std::cout << "Graph generated in SVG format: " << svg_filename << std::endl;

    // Read the SVG file contents
    std::ifstream svg_file(svg_filename);
    if (!svg_file.is_open())
    {
        std::cerr << "Error: Unable to open SVG file" << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << svg_file.rdbuf();
    svg_file.close();

    // Initialize the HTML viewer if it doesn't exist yet
    if (!html_viewer)
    {
        html_viewer = std::make_shared<HTMLViewer>();
    }

    // Display the SVG in the viewer and pass the temporary file path to be deleted on close
    html_viewer->loadAndDisplaySVG(buffer.str(), svg_filename);
}
#endif // HAVE_GTK_WEBKIT