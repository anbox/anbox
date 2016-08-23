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
#ifndef RANGE_H
#define RANGE_H

#include <vector>

class Range {

public:
    Range():m_start(0),m_end(0),m_size(0){};
    Range(int start,int size):m_start(start),m_end(start+size),m_size(size){};
    Range(const Range& r):m_start(r.m_start),m_end(r.m_end),m_size(r.m_size){};
    void setRange(int start,int size){m_start = start; m_end = start+size; m_size = size;};
    inline int getStart() const{return m_start;};
    inline int getEnd() const{return m_end;};
    inline int getSize() const{return m_size;};
    Range& operator=(const Range& r) {
        m_start = r.m_start;
        m_end = r.m_end;
        m_size = r.m_size;
        return *this;
    }
    bool operator ==(const Range& r) const {
        return m_start == r.m_start && m_size == r.m_size && m_end == r.m_end;
    }
    bool operator !=(const Range& r) const {return !((*this) == r);};
    bool rangeIntersection(const Range& r,Range& rOut) const ;
    bool rangeUnion(const Range& r,Range& rOut) const ;

private:
    int m_start;
    int m_end;
    int m_size;
};

class RangeList {
public:
      void addRange(const Range& r);
      void addRanges(const RangeList& rl);
      void delRange(const Range& r,RangeList& deleted);
      void delRanges(const RangeList& rl,RangeList& deleted);
      bool empty() const;
      void merge();
      int  size() const;
      void clear();
      Range& operator[](unsigned int i){return list[i];};
private:
  void erase(unsigned int i);
  std::vector<Range> list;
};




#endif
