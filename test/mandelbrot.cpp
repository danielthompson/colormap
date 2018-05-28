#include "colormap.hpp"
#include "grid.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>


int main () {
    // set up the grid: 701 x 401 grid points (pixels) representing the region
    // [-2.5, 1] x [-1, 1].
    grid<2, major_order::col> g { {701, {-2.5, 1.}}, {401, {-1., 1.}} };
    using grid_point_t = typename decltype(g)::grid_point_type;

    // set up the functor to be visualized on the grid, e.g. the mandelbrot set
    auto mandelbrot = [] (grid_point_t c_arr) {
        // constants 
        const size_t max_it = 1000;
        const double bail_out = std::pow(2, 16);

        // rescaling function
        auto func = [] (double x) { return pow(x, 0.1); };

        std::complex<double> c { c_arr[0], c_arr[1] };
        std::complex<double> z = 0.;
        size_t it;
        for (it = 0; it < max_it && std::norm(z) < bail_out; ++it)
            z = z * z + c;
        if (it < max_it) {
            double log2_z = log(std::norm(z)) * 0.5 / log(2);
            double nu = log(log2_z) / log(2);
            return func(it + 1 - nu);
        } else {
            return 0.;
        }
    };

    // Iterator adapter mapping grid points to function values.
    // Result is a `mapped` object that behave like a container.
    auto val_map = itadpt::map(g, mandelbrot);

    // collect the function values -- not required but faster here
    std::vector<double> val;
    std::copy(val_map.begin(), val_map.end(), std::back_inserter(val));

    // find the maximum value
    double max = *std::max_element(val.begin(), val.end());

    // get a colormap and rescale it
    auto pal = color::palettes.at("inferno").rescale(1, max);
    // and use it to map the values to colors
    auto pix = itadpt::map(val, pal);

    // Construct a PPM object from the `mapped` object `pix`. Color space is
    // inferred from color type of pix. "inferno" is an RGB palette, so `pmap`
    // will represent a PPM image. For a grayscale colormap, it would result in
    // a PGM image.
    color::pixmap<decltype(pix.begin())> pmap(pix.begin(), g.shape());

    /* binary output */ {
        std::ofstream os("appleman_binary." + pmap.file_extension(),
                         std::ios_base::binary);
        pmap.write_binary(os);
    }
    /* ASCII text output */{
        std::ofstream os("appleman_ascii." + pmap.file_extension());
        pmap.write_ascii(os);
    }

}
