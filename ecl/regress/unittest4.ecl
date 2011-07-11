/*##############################################################################

    Copyright (C) 2011 HPCC Systems.

    All rights reserved. This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
############################################################################## */

r := RECORD
  UNSIGNED4 id;
  STRING50 name;
  STRING100 address;
END;

r2 := RECORD(r)
  STRING extra{maxlength(1000)}
END;

ds := dataset('ds', r, thor);

ASSERT(RANDOM() != 0, CONST);

ASSERT(SIZEOF(R) = 154);
ASSERT(SIZEOF(R2,MAX) = 1158);
ASSERT(SIZEOF(R) = 154, CONST);
ASSERT(SIZEOF(R) = 154, CONST);


//Not supported as a generate time constant - too many complexities with ifblocks/var length etc
ASSERT(SIZEOF(ds) = 154);
ASSERT(SIZEOF(recordof(ds)) = 154, const);  // but this is
