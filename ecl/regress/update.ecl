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

personRecord := RECORD
unsigned4 personid;
string1 sex;
string33 forename;
string33 surname;
unsigned8 filepos{virtual(fileposition)};
    END;

personDataset := DATASET('person',personRecord,thor);
i := index(personDataset, { personDataset }, 'i');

output(personDataset,,'abc',update);
build(i,update);


s := sort(personDataset, surname);
x1 := s : persist('persist::x1');
d := dedup(s, surname);

s2 := sort(d, forename, surname);
output(s2+x1,,'abcUpdate',update);
output(s2+x1,,'abcPlain');
output(s2+x1,,'abcOverwrite',overwrite);
output(s2+x1,,'abcNoOverwrite',nooverwrite);