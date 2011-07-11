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


#option ('targetClusterType', 'roxie');

r1 :=        RECORD
unsigned4        r1f1;
string20         r1f2;
            END;

r2 :=        RECORD
unsigned4        r2f1;
dataset(r1)      r2f2;
            END;

r3 :=        RECORD
unsigned4        r3f1;
r2               r3f2;
r2               r3f3;
unsigned4        r3f4;
            END;

d := DATASET('d3', r3, FLAT);

i := index(d, { r3f1, r3f4 }, { d }, 'i');

//Try and trigger a row being serialized to the slave, and requiring serialization
k := JOIN(d, i, KEYED(left.r3f1 = right.r3f1) and (left.r3f2 = right.r3f2) and (left.r3f4= right.r3f4));

output(k);
