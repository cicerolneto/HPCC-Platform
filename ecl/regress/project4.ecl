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

#option ('globalOptimize', 1);

ppersonRecord := RECORD
string10    surname ;
string10    forename;
string2     nl;
  END;


ppersonRecordEx := RECORD
            ppersonRecord;
unsigned4   id;
unsigned1   id1;
unsigned1   id2;
unsigned1   id3;
    END;


pperson := DATASET('in.d00', ppersonRecord, FLAT);


ppersonRecordEx projectFunction(ppersonRecord incoming) := 
    TRANSFORM
        SELF.id := random();
        SELF.id1 := 0;
        SELF.id2 := 0;
        SELF.id3 := 0;
        SELF := incoming;
    END;


ppersonEx := project(pperson, projectFunction(left));

ppersonRecordEx projectFunction2(ppersonRecordEx l) := 
    TRANSFORM,skip(l.id=0)
        SELF.id1 := (l.id DIV 10000) % 100;
        SELF.id2 := (l.id DIV 100) % 100;
        SELF.id3 := (l.id) % 100;
        SELF := l;
    END;


ppersonEx2 := project(ppersonEx, projectFunction2(left));

output(ppersonEx2(id2>0), , 'out.d00');



