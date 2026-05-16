//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef NUMGENERATOR_H_
#define NUMGENERATOR_H_

namespace flora { // override çalışmamasının sebebi flora namespace içinde olması gerektiğindenmiş. FLoRa namespace'ine aldım

class NumGenerator {
public:
    NumGenerator();
    virtual ~NumGenerator();
    NumGenerator(const NumGenerator &other);

    static double exponential(double lambda);
};

} // namespace flora içinde artık override sorunu olmayacak
#endif
