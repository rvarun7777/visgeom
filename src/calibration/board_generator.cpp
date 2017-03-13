/*
This file is part of visgeom.

visgeom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

visgeom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with visgeom.  If not, see <http://www.gnu.org/licenses/>.
*/ 

#include "calibration/board_generator.h"

#include "std.h"
#include "io.h"
#include "ocv.h"
#include "eigen.h"

#include "projection/generic_camera.h"

BoardGenerator::BoardGenerator(const ICamera * camera, int Nx, int Ny,
        double cellSize, double factor):
            _camera(camera->clone()),
            _Nx(Nx),
            _Ny(Ny),
            _cellSize(cellSize) {}
            
void BoardGenerator::generate(Mat8u & dst, const Transf xi) const
{
    dst.create(_camera->height, _camera->width);
    Matrix3d R = xi.rotMat();
    Vector3d ex = R.col(0);
    Vector3d ey = R.col(1);
    Vector3d ez = R.col(2);
    const double lambdaNum = xi.trans().dot(ez);
    
    for (int r = 0; r < _camera->height; r++)
    {
        double x, y;
        for (int c = 0; c < _camera->width; c++)
        {
            Vector2d pt(c, r);
            Vector3d dir;
            if (not _camera->reconstructPoint(pt, dir)) 
            {
                dst(r, c) = 0;
                continue;
            }
            const double lambdaDenom = dir.dot(ez);
            const double lambda = lambdaNum / lambdaDenom;
            if (lambda < 0)
            {
                dst(r, c) = 255;
                continue;
            }
            Vector3d v = dir * lambda - xi.trans();
            x = v.dot(ex);
            y = v.dot(ey);
            const double smoothx = 0.5 * M_PI * lambda / (ex[0] * _camera->width);
            const double smoothy = 0.5 * M_PI * lambda / (ey[1] * _camera->width);
            dst(r, c) = computeBrightness(x, y, min(smoothx, smoothy));
        }
    }
    //reconstruct
    //find the intersection
    //find the color
}

uchar BoardGenerator::computeBrightness(const double x, const double y, const double smooth) const
{
    if (x <= -_cellSize or x >= _Nx * _cellSize) return 255;
    else if (y <= -_cellSize or y >= _Ny * _cellSize) return 255;
    
    const double xrel = x / _cellSize;
    const double yrel = y / _cellSize;
    int cx = ceil(xrel);
    int cy = ceil(yrel);
    
    const double dx = abs(xrel - round(xrel));
    const double dy = abs(yrel - round(yrel));
    const double delta = min(dx, dy);
    if (cx % 2 == cy % 2)
    {
        if (cx != 0 and cy != 0 and cx != _Nx and cy != _Ny)
        {
            const double res = 180*(1 + delta * _cellSize / smooth);
            return uchar(min(res, 255.));
        }
        else return 255;
    }
    else
    {
        const double res = 180*(1 - delta * _cellSize / smooth);
        return uchar(max(res, 0.));
    }
//    double res = 1 - sin(x/_cellSize * M_PI) * sin(y/_cellSize * M_PI)  * _factor;
//    res = max( min(1., (res)), 0. );
//    return uchar(res * 255); 
}


    
