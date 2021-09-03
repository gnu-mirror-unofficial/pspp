/* PSPP - a program for statistical analysis.
   Copyright (C) 2009, 2011 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <config.h>

#include "output/charts/np-plot.h"

#include "data/case.h"
#include "data/casereader.h"
#include "math/np.h"
#include "output/cairo-chart.h"

#include "gettext.h"
#define _(msgid) gettext (msgid)

static void
np_plot_chart_draw (const struct chart *chart, cairo_t *cr,
                    struct xrchart_geometry *geom)
{
  const struct np_plot_chart *npp = to_np_plot_chart (chart);
  struct casereader *data;
  struct ccase *c;

  xrchart_write_title (cr, geom, _("Normal Q-Q Plot of %s"), chart->title);
  xrchart_write_xlabel (cr, geom, _("Observed Value"));
  xrchart_write_ylabel (cr, geom, _("Expected Normal"));
  if (! xrchart_write_xscale (cr, geom,
                      npp->x_lower - npp->slack,
                              npp->x_upper + npp->slack))
    return;

  if (! xrchart_write_yscale (cr, geom, npp->y_first, npp->y_last))
    return;

  data = casereader_clone (npp->data);
  for (; (c = casereader_read (data)) != NULL; case_unref (c))
    xrchart_datum (cr, geom, 0,
                   case_num_idx (c, NP_IDX_Y),
                   case_num_idx (c, NP_IDX_NS));
  casereader_destroy (data);

  xrchart_line (cr, geom, npp->slope, npp->intercept,
              npp->y_first, npp->y_last, XRCHART_DIM_Y);
}

static void
dnp_plot_chart_draw (const struct chart *chart, cairo_t *cr,
                     struct xrchart_geometry *geom)
{
  const struct np_plot_chart *dnpp = to_np_plot_chart (chart);
  struct casereader *data;
  struct ccase *c;

  xrchart_write_title (cr, geom, _("Detrended Normal Q-Q Plot of %s"), chart->title);
  xrchart_write_xlabel (cr, geom, _("Observed Value"));
  xrchart_write_ylabel (cr, geom, _("Dev from Normal"));
  if (! xrchart_write_xscale (cr, geom, dnpp->y_min, dnpp->y_max))
    return;
  if (! xrchart_write_yscale (cr, geom, dnpp->dns_min, dnpp->dns_max))
    return;

  data = casereader_clone (dnpp->data);
  for (; (c = casereader_read (data)) != NULL; case_unref (c))
    xrchart_datum (cr, geom, 0, case_num_idx (c, NP_IDX_Y),
                   case_num_idx (c, NP_IDX_DNS));
  casereader_destroy (data);

  xrchart_line (cr, geom, 0, 0, dnpp->y_min, dnpp->y_max, XRCHART_DIM_X);
}

void
xrchart_draw_np_plot (const struct chart *chart, cairo_t *cr,
                      struct xrchart_geometry *geom)
{
  const struct np_plot_chart *npp = to_np_plot_chart (chart);

  if (npp->detrended)
    dnp_plot_chart_draw (chart, cr, geom);
  else
    np_plot_chart_draw (chart, cr, geom);
}
