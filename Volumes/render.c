#include  <mni.h>

private  void  clip(
    Real  left_edge,
    Real  right_edge,
    Real  origin,
    Real  delta,
    Real  *t_min,
    Real  *t_max )
{
    Real  t;

    if( delta == 0.0 )
    {
        if( origin < left_edge || origin >= right_edge )
        {
            *t_min = 1.0;
            *t_max = 0.0;
        }
    }
    else if( delta > 0.0 )
    {
        t = (left_edge - origin) / delta;
        if( t > *t_min )
            *t_min = t;

        t = (right_edge - origin) / delta;
        if( t < *t_max )
            *t_max = t;
    }
    else
    {
        t = (left_edge - origin) / delta;
        if( t < *t_max )
            *t_max = t;

        t = (right_edge - origin) / delta;
        if( t > *t_min )
            *t_min = t;
    }
}

public  void  render_volume_to_slice(
    int             n_dims1,
    int             sizes1[],
    void            *volume_data1,
    Data_types      volume1_type,
    int             n_slices1,
    Real            weights1[],
    int             strides1[],
    Real            **origins1,
    Real            x_axis1[],
    Real            y_axis1[],
    int             n_dims2,
    int             sizes2[],
    void            *volume_data2,
    Data_types      volume2_type,
    int             n_slices2,
    Real            weights2[],
    int             strides2[],
    Real            **origins2,
    Real            x_axis2[],
    Real            y_axis2[],
    BOOLEAN         interpolation_flag,    /* ARGSUSED */
    unsigned short  **cmode_colour_map,
    Colour          **rgb_colour_map,
    Colour          empty_colour,
    pixels_struct   *pixels )
{
    int   i, c, p, total_cases, case_index, case_multiplier, offset, int_start;
    int   s, x, y, n_cases1[MAX_DIMENSIONS];
    int   **x_offsets1, **y_offsets1, *start_x, *end_x;
    int   ***which_x_offsets1;
    int   remainder_case, x_size, y_size;
    Real  start_c, x_start, x_end, remainder, tmp_origin[MAX_DIMENSIONS];
    Real  remainder_offset, left_edge, right_edge, delta;
    static  int   max_cases[MAX_DIMENSIONS] = { 10, 10, 4, 3, 3 };

    x_size = pixels->x_size;
    y_size = pixels->y_size;

    total_cases = 1;
    for_less( c, 0, n_dims1 )
    {
        delta = ABS( x_axis1[c] );
        if( delta <= 1.0 / (Real) max_cases[n_dims1-1] )
            n_cases1[c] = max_cases[n_dims1-1];
        else
            n_cases1[c] = (int) (1.0 / delta) + 1;

        total_cases *= n_cases1[c];
    }

    ALLOC2D( x_offsets1, total_cases, x_size );

    for_less( i, 0, total_cases )
    {
        p = i;
        for_less( c, 0, n_dims1 )
        {
            case_index = p % n_cases1[c];
            tmp_origin[c] = ((Real) case_index + 0.5) / (Real) n_cases1[c];
            p /= n_cases1[c];
        }

        for_less( x, 0, x_size )
        {
            offset = 0;
            for_less( c, 0, n_dims1 )
            {
                start_c = tmp_origin[c] + (Real) x * x_axis1[c];
                offset += strides1[c] * FLOOR( start_c );
            }
            x_offsets1[i][x] = offset;
        }
    }

    ALLOC2D( y_offsets1, n_slices1, y_size );
    ALLOC2D( which_x_offsets1, n_slices1, y_size );
    ALLOC( start_x, y_size );
    ALLOC( end_x, y_size );

    for_less( y, 0, y_size )
    {
        x_start = 0.0;
        x_end = (Real) (x_size - 1);
        for_less( s, 0, n_slices1 )
        {
            offset = 0;
            case_index = 0;
            case_multiplier = 1;
            for_less( c, 0, n_dims1 )
            {
                start_c = origins1[s][c] + (Real) y * y_axis1[c] + 0.5;
                int_start = FLOOR( start_c );
                remainder = start_c - int_start;
                remainder_case = (int) (remainder * n_cases1[c]);
                remainder_offset = remainder -
                                   (remainder_case + 0.5)/ n_cases1[c];
                case_index += case_multiplier * remainder_case;
                case_multiplier *= n_cases1[c];
                offset += strides1[c] * int_start;

                left_edge = 0.0;
                right_edge = (Real) sizes1[c];

                if( remainder_offset < 0.0 )
                    right_edge += remainder_offset;
                else
                    left_edge += remainder_offset;

                clip( left_edge, right_edge, start_c, x_axis1[c],
                      &x_start, &x_end );
            }

            y_offsets1[s][y] = offset;
            which_x_offsets1[s][y] = x_offsets1[case_index];
        }

        start_x[y] = CEILING( x_start );
        end_x[y] = FLOOR( x_end );

        if( start_x[y] > x_size )
            start_x[y] = x_size;
        if( end_x[y] < 0 )
            end_x[y] = 0;
        if( start_x[y] > end_x[y] + 1 )
            start_x[y] = end_x[y] + 1;
    }

    if( pixels->pixel_type == RGB_PIXEL )
    {
#include  "render_include1.c"
    }
    else
    {
#define   COLOUR_MAP
#include  "render_include1.c"
    }

    FREE2D( y_offsets1 );
    FREE2D( x_offsets1 );
    FREE2D( which_x_offsets1 );
    FREE2D( origins1 );
    FREE( start_x );
    FREE( end_x );
}
