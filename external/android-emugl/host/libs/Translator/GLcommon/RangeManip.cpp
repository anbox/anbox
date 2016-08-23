/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include <GLcommon/RangeManip.h>


bool Range::rangeIntersection(const Range& r,Range& rOut) const {
    if(m_start > r.getEnd() || r.getStart() > m_end) return false;
    int max_start = (m_start > r.getStart())? m_start:r.getStart();
    int min_end = (m_end < r.getEnd())?m_end:r.getEnd();
    int size = min_end - max_start;
    if(size) {
        rOut.setRange(max_start,min_end-max_start);
        return true;
    }
    return false;
}

bool Range::rangeUnion(const Range& r,Range& rOut) const {
    if(m_start > r.getEnd() || r.getStart() > m_end) return false;
    int min_start = (m_start < r.getStart())?m_start:r.getStart();
    int max_end = (m_end > r.getEnd())?m_end:r.getEnd();
    int size =  max_end - min_start;
    if(size) {
        rOut.setRange(min_start,max_end-min_start);
        return true;
    }
    return false;
}

void RangeList::addRange(const Range& r) {
    if(r.getSize())
        list.push_back(r);
}

void RangeList::addRanges(const RangeList& rl) {
    for(int i =0; i< rl.size();i++) {
       addRange(rl.list[i]);
    }
}

void RangeList::delRanges(const RangeList& rl,RangeList& deleted) {
    for(int i =0; i< rl.size();i++) {
       delRange(rl.list[i],deleted);
    }
}

bool RangeList::empty() const{
    return list.empty();
}

int  RangeList::size() const{
    return list.size();
}
void RangeList::clear() {
    return list.clear();
}

void RangeList::erase(unsigned int i) {
    if(i > list.size()) return;
    list.erase(list.begin() +i);
}

void RangeList::delRange(const Range& r,RangeList& deleted) {
    if(r.getSize() == 0) return;

    Range intersection;
    Range temp;
    // compare new rect to each and any of the rects on the list
    for (int i=0;i<(int)list.size();i++) { // i must be signed for i-- below
     // if there is intersection
     if (r.rangeIntersection(list[i],intersection)) {
             Range old=list[i];
         // remove old as it is about to be split
         erase(i);
         i--;
         if (intersection!=old) { // otherwise split:
             //intersection on right side
             if(old.getStart() != intersection.getStart()) {
                 list.insert(list.begin(),Range(old.getStart(),intersection.getStart() - old.getStart()));
             }

             //intersection on left side
             if(old.getEnd() != intersection.getEnd()) {
                 list.insert(list.begin(),Range(intersection.getEnd(),old.getEnd() - intersection.getEnd()));
             }
         }
         deleted.addRange(intersection);
     }
 }
}

void RangeList::merge() {
    if(list.empty()) return;

    Range temp;
    bool changed;

    do { // re-run if changed in last run
        changed=0;
        // run for each combinations of two rects in the list
        for (int i=0;i<(((int)list.size())-1) && !changed ;i++)
        {
            for (int j=i+1;j<(int)list.size() && !changed ;j++)
            {
               if (list[i].rangeUnion(list[j],temp)) {
                    // are them exactly one on left of the other
                    list[i] = temp;
                    erase(j);
                    changed=1;
               }
            }
        }
    } while (changed);
}
