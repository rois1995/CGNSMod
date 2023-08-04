#include <stdio.h>
#include <vector>
#include <numeric>
#include <algorithm>
#include <string>
#include <unordered_set>
#include <unordered_map>
/* cgnslib.h file must be located in directory specified by -I during compile: */
#include "cgnslib.h"
#include <iostream>
#include <execinfo.h>
#include <cstdlib>
#include <unistd.h>  // Added this include for STDERR_FILENO

using namespace std;

struct Point3DExt {
    float x;
    float y;
    float z;
    
    long int totalIndex, zoneIndex, localIndex;

    Point3DExt(float _x, float _y, float _z, int _totInd, int _zoneInd, int _locInd) : x(_x), y(_y), z(_z), totalIndex(_totInd), zoneIndex(_zoneInd), localIndex(_locInd) {}
    Point3DExt() : x(0), y(0), z(0), totalIndex(0), zoneIndex(0), localIndex(0) {}
    Point3DExt(const Point3DExt& copy): x(copy.x), y(copy.y), z(copy.z), 
                               totalIndex(copy.totalIndex), zoneIndex(copy.zoneIndex), 
                               localIndex(copy.localIndex) {}
    Point3DExt(int _zoneInd, cgsize_t _locInd): x(0.0), y(0.0), z(0.0), 
                               totalIndex(0), zoneIndex(_zoneInd), 
                               localIndex(_locInd) {}

    void print(){
        cout << "Point with totalIndex = " << totalIndex << " , zoneIndex = " << zoneIndex;
        cout << " , localIndex = " << localIndex << " , has coordinates {";
        cout << x << ", " << y << ", " << z << "}" << endl;
    }

    bool operator==(const Point3DExt& other) const {
        // cout << "zoneIndex " << zoneIndex << " vs " << other.zoneIndex;
        // cout << "   localIndex " << localIndex << " vs " << other.localIndex << endl;
        // if(zoneIndex == other.zoneIndex && localIndex == other.localIndex) cout << "sono uguali cazzo!" << endl;
        return (zoneIndex == other.zoneIndex && localIndex == other.localIndex);
    }
};

struct Point3D {
    float x;
    float y;
    float z;

    long int totalIndex, zoneIndex, localIndex;

    Point3D(float _x, float _y, float _z, int _totInd, int _zoneInd, int _locInd) : x(_x), y(_y), z(_z), totalIndex(_totInd), zoneIndex(_zoneInd), localIndex(_locInd) {}
    Point3D() : x(0), y(0), z(0), totalIndex(0), zoneIndex(0), localIndex(0) {}
    Point3D(const Point3D& copy): x(copy.x), y(copy.y), z(copy.z), 
                               totalIndex(copy.totalIndex), zoneIndex(copy.zoneIndex), 
                               localIndex(copy.localIndex) {}
    Point3D(const Point3DExt& copy): x(copy.x), y(copy.y), z(copy.z), 
                               totalIndex(copy.totalIndex), zoneIndex(copy.zoneIndex), 
                               localIndex(copy.localIndex) {}
    bool operator==(const Point3D& other) const {
        return (x == other.x && y == other.y && z == other.z);
    }
    void print(){
        cout << "Point with totalIndex = " << totalIndex << " , zoneIndex = " << zoneIndex;
        cout << " , localIndex = " << localIndex << " , has coordinates {";
        cout << x << ", " << y << ", " << z << "}" << endl;
    }
};



struct Element{


    cgsize_t totalIndex; // starts from 0
    int zoneIndex; // starts from 1
    int sectionIndex; // starts from 1
    cgsize_t localIndex; // starts from 1
    cgsize_t localLocalIndex; // starts from 1

    ElementType_t type;

    bool volumeElem;


    public:
    vector<cgsize_t> indexPoints;

    Element(vector<cgsize_t> _ind, cgsize_t _totInd, int _zoneInd, int _secInd, cgsize_t _locInd, cgsize_t _loclocInd, ElementType_t _type) : 
            indexPoints(_ind), totalIndex(_totInd), zoneIndex(_zoneInd), 
            sectionIndex(_secInd), localIndex(_locInd), localLocalIndex(_loclocInd), type(_type), volumeElem(isVolume()) {}
    Element() : indexPoints(vector<cgsize_t>(3, 0)), totalIndex(0), zoneIndex(0),  
                sectionIndex(0), localIndex(0), localLocalIndex(0), type(TRI_3), volumeElem(false) {}
    // Assignment operator
    Element& operator=(const Element& other) {
        if (this != &other) {  // protect against invalid self-assignment
            totalIndex = other.totalIndex;
            zoneIndex = other.zoneIndex;
            sectionIndex = other.sectionIndex;
            localIndex = other.localIndex;
            localLocalIndex = other.localLocalIndex;
            type = other.type;
            volumeElem = other.volumeElem;
            indexPoints.resize(other.indexPoints.size());
            indexPoints = other.indexPoints;
        }
        // by convention, always return *this
        return *this;
    }
    bool isVolume() {
        switch (this->type) {
            case TRI_3: 
            case QUAD_4:
                return false;
            case TETRA_4:
            case PYRA_5: 
            case PENTA_6:
            case HEXA_8:
                return true;
            default: 
                printf("Elemento %s non riconosciuto. Exiting..", ElementTypeName[type]);
                return false;

        }
    }

    // Used to search for boundary elements in here
    bool operator==(const Element& other) const {
        return ( zoneIndex == other.zoneIndex && localLocalIndex == other.localLocalIndex);
    }

};

namespace std {
    template <>
    struct hash<Element> {
        size_t operator()(const Element& elem) const {
            size_t h1 = std::hash<int>{}(elem.zoneIndex);
            size_t h2 = std::hash<int>{}(elem.localLocalIndex);
            return h1^h2;
        }
    };


    template <>
    struct hash<Point3D> {
        size_t operator()(const Point3D& point) const {
            size_t h1 = std::hash<float>{}(point.x);
            size_t h2 = std::hash<float>{}(point.y);
            size_t h3 = std::hash<float>{}(point.z);
            return h1 ^ h2 ^ h3;
        }
    };


    template <>
    struct hash<Point3DExt> {
        size_t operator()(const Point3DExt& point) const {
            size_t h1 = std::hash<int>{}(point.zoneIndex);
            size_t h2 = std::hash<int>{}(point.localIndex);
            return h1^h2;
        }
    };
}

// void removeDuplicates(std::vector<Point3D>& coordinates, std::unordered_map<size_t, size_t>& removedIndices, std::unordered_set<Point3D>& uniqueCoordinates, vector<Point3D>& globalUniquePoints, std::unordered_set<Point3DExt>& DummyMapPoints) {
    
//     globalUniquePoints.resize(coordinates.size());
//     // Iterate through the vector of coordinates
//     auto it = coordinates.begin();
//     size_t currentIndex = 0;
//     long int uniqueIndex = 0;
//     while (it != coordinates.end()) {
//         auto findDup = uniqueCoordinates.find(*it);
//         // Check if the current coordinate is already in the set
//         if ( findDup != uniqueCoordinates.end()) {
//             // Store the removed index and its corresponding original index in the map
//             removedIndices[currentIndex] = findDup->totalIndex;
//             // cout << "Removed: x = " << it->x << " y = " << it->y << " z = " << it->z << endl;
//             // cout << "Checked: x = " << findDup->x << " y = " << findDup->y << " z = " << findDup->z << endl << endl;
//             // Erase the duplicate coordinate from the vector
//             // it = coordinates.erase(it);
//         } else {
//             // Add the coordinate to the set and continue to the next coordinate
//             auto cose = *it;
//             uniqueCoordinates.insert(cose);
//             cose.totalIndex = uniqueIndex;
//             globalUniquePoints[uniqueIndex] = cose;
//             Point3DExt coseExt(cose);
//             DummyMapPoints.insert(coseExt);
//             uniqueIndex++;
//         }
//         ++it;
//         ++currentIndex;
//     }

//     globalUniquePoints.resize(uniqueIndex);
// }


int extractPointsAndElements(string filename, std::unordered_set<Point3D>& coordinates, vector<vector<long int>>& PointDict, std::unordered_set<Element>& Elements, std::unordered_set<Point3DExt>& DummyMapPoints) {

    int nzones;
    int index_file;

    if (cg_open(filename.c_str(),CG_MODE_READ,&index_file)) cg_error_exit();
    if (cg_nzones(index_file, 1, &nzones)) cg_error_exit();

    vector<vector<double>> xCoords(nzones, vector<double>());
    vector<vector<double>> yCoords(nzones, vector<double>());
    vector<vector<double>> zCoords(nzones, vector<double>());
    PointDict.resize(nzones, vector<long int>());

    vector<vector<vector<ElementType_t>>> ElementTypes(nzones, vector<vector<ElementType_t>>());
    vector<vector<vector<cgsize_t>>> ElementConnectivity(nzones, vector<vector<cgsize_t>>());

    vector<vector<int>> nSectionsAll(nzones, vector<int>());
    vector<vector<int>> VolumeSections(nzones, vector<int>());
    vector<vector<int>> SurfaceSections(nzones, vector<int>());
    vector<vector<string>> SectionsNames(nzones, vector<string>());
    
    long int OldPointSize = 0;
    long int OldElemSize = 0;
    long int globaIndex = 0;

    for (int index_zone = 1; index_zone <= nzones; index_zone++)
        {
            printf("\nReading zone %d data...\n", index_zone);

            // Read number of nodes and of elements
            cgsize_t sizes[2], one=1;
            char zoneName[33], secName[33];
            cg_zone_read(index_file, 1, index_zone, zoneName, sizes);


            // Resize vectors
            xCoords[index_zone-1].resize(sizes[0]);
            yCoords[index_zone-1].resize(sizes[0]);
            zCoords[index_zone-1].resize(sizes[0]);

            PointDict[index_zone-1].resize(sizes[0]);

            long int NewPointSize = OldPointSize+sizes[0];


            printf("\nzone %d, named %s, has %d nodes and %d elements \n", index_zone, zoneName, sizes[0], sizes[1]);


            // Read coordinates
            cg_coord_read(index_file, 1, index_zone, "CoordinateX", RealDouble, &one, &sizes[0], xCoords[index_zone-1].data());
            cg_coord_read(index_file, 1, index_zone, "CoordinateY", RealDouble, &one, &sizes[0], yCoords[index_zone-1].data());
            cg_coord_read(index_file, 1, index_zone, "CoordinateZ", RealDouble, &one, &sizes[0], zCoords[index_zone-1].data());

            for (long int iPoint = 0; iPoint < sizes[0]; iPoint++) {
                Point3DExt pointExt(xCoords[index_zone-1][iPoint], yCoords[index_zone-1][iPoint], zCoords[index_zone-1][iPoint], OldPointSize+iPoint, index_zone, iPoint+1);
                Point3D point(pointExt);
                coordinates.insert(point);
                DummyMapPoints.insert(pointExt);
                PointDict[index_zone-1][iPoint] = OldPointSize+iPoint;
            }
            
            int nsections;

            if ( cg_nsections(index_file,1,index_zone,&nsections) ) cg_error_exit();
            printf("\nnumber of sections=%i\n",nsections);

            ElementTypes[index_zone-1].resize(nsections);
            ElementConnectivity[index_zone-1].resize(nsections);
            nSectionsAll[index_zone-1].resize(nsections);
            SectionsNames[index_zone-1].resize(nsections);

            for (int index_sect=1; index_sect <= nsections; index_sect++)
            {
                ElementType_t type;
                int nBoundary, parentFlag;
                cgsize_t eBeg, eEnd;

                cg_section_read(index_file, 1, index_zone, index_sect, secName, &type, &eBeg, &eEnd, &nBoundary, &parentFlag);

                int NElements = (int)eEnd-(int)eBeg+1;  

                OldElemSize = Elements.size();
                int NewElemSize = OldElemSize+NElements;
                printf("New elements size %d\n", NewElemSize);
                // Elements.resize(NewElemSize); 

                printf("\nReading section data...\n");
                printf("   section %d is called %s\n", index_sect, secName);
                printf("   section type = %s\n",ElementTypeName[type]);
                printf("   N elements = %d\n", NElements);
                printf("   istart,iend = %i, %i\n",(int)eBeg,(int)eEnd);
                printf("   nboundary = %d\n", nBoundary);

                SectionsNames[index_zone-1][index_sect-1] = secName;
                cout << SectionsNames[index_zone-1][index_sect-1] << endl;

                int NPoints = 0;

                switch (type) {
                    case TRI_3:  
                        NPoints = 3;
                        ElementTypes[index_zone-1][index_sect-1].resize(NElements, TRI_3);
                        SurfaceSections[index_zone-1].push_back(index_sect);
                        break;

                    case QUAD_4:
                        NPoints = 4;
                        ElementTypes[index_zone-1][index_sect-1].resize(NElements, QUAD_4);
                        SurfaceSections[index_zone-1].push_back(index_sect);
                        break;
                    
                    case TETRA_4:
                        NPoints = 4;
                        ElementTypes[index_zone-1][index_sect-1].resize(NElements, TETRA_4);
                        VolumeSections[index_zone-1].push_back(index_sect);
                        break;

                    case PYRA_5: 
                        NPoints = 5;               
                        ElementTypes[index_zone-1][index_sect-1].resize(NElements, PYRA_5);
                        VolumeSections[index_zone-1].push_back(index_sect);
                        break;
                    
                    case PENTA_6:
                        NPoints = 6;
                        ElementTypes[index_zone-1][index_sect-1].resize(NElements, PENTA_6);
                        VolumeSections[index_zone-1].push_back(index_sect);
                        break;
                    
                    case HEXA_8:
                        NPoints = 8;
                        ElementTypes[index_zone-1][index_sect-1].resize(NElements, HEXA_8);
                        VolumeSections[index_zone-1].push_back(index_sect);
                        break;

                    default: 
                        printf("Elemento %s non riconosciuto. Exiting..", ElementTypeName[type]);
                        return 1;

                }

                ElementConnectivity[index_zone-1][index_sect-1].resize(NPoints*NElements);

                cg_elements_read(index_file, 1, index_zone, index_sect, ElementConnectivity[index_zone-1][index_sect-1].data(), NULL);

                printf("Saving elements for section %d...\n", index_sect);

                for (int iElem = 0; iElem < NElements; iElem++){
                    // printf("%d\n", iElem);
                    vector<cgsize_t>::const_iterator first = ElementConnectivity[index_zone-1][index_sect-1].begin() + iElem*NPoints;
                    vector<cgsize_t>::const_iterator last = ElementConnectivity[index_zone-1][index_sect-1].begin() + iElem*NPoints + NPoints;
                    vector<cgsize_t> Points(first, last);
                    
                    Element ElementHere(Points, globaIndex, index_zone, index_sect, iElem+1, (int)eBeg+iElem, type);
                    Elements.insert(ElementHere);
                    globaIndex++;
                }
                
            }


            OldPointSize = NewPointSize;
        }

    return 0;

}   


void removeAbuttingElements(std::unordered_set<Element>& Elements2Remove, std::unordered_set<Element>& UniqueElements){

    // Idea: Cycle on all the elements and remove those that belong to an overlapping boundary

    // First allocate vector of unique elements to have same size of total element vector
    cout << "Original number of elements " << UniqueElements.size() << endl;
    auto it = Elements2Remove.begin();
    size_t NewTotalIndex = 0;
    while (it != Elements2Remove.end()) {
        
        auto cose = *it;
        if(!cose.volumeElem){
            // cout << "Element " << it->totalIndex << " belongs to zone " << it->zoneIndex;
            // cout << " has type " << it->type << " and local local index ";
            // cout << it->localLocalIndex << endl;

            auto foundIt = UniqueElements.find(*it);

            // cout << "index = " << std::distance(Elements2Remove.begin(), foundIt) << " while total number is ";
            // cout << std::distance(Elements2Remove.begin(), Elements2Remove.end()) << endl;

            if ( foundIt != UniqueElements.end()) {
                // Add the coordinate to the set and continue to the next coordinate
                // cout << "RemovePoints Has following list of points: " << endl;;
                // for(int i = 0; i < foundIt->indexPoints.size(); i++){
                //     cout << foundIt->zoneIndex << " " << foundIt->indexPoints[i] << endl;
                // }
                // cout << endl;
                UniqueElements.erase(foundIt);
            }
        }
               
        ++it;
    }

    // Hopefully this should reduce the size of the vector to the actual inserted elements
    // UniqueElements.shrink_to_fit();
    cout << "New number of elements " << UniqueElements.size() << endl;

    // Now adjust the global index of the elements
    std::unordered_set<Element> dummyUniqueElements;
    int nNonDupElems = 0;
    auto itNow = UniqueElements.begin();
    while (itNow != UniqueElements.end()) {
        auto cose = *itNow;
        cose.totalIndex = nNonDupElems;
        dummyUniqueElements.insert(cose);
        ++itNow;
        nNonDupElems++;
    }

    UniqueElements.swap(dummyUniqueElements);

}

void assignNewPointIndices(std::unordered_set<Point3DExt>& DummyMapPoints, vector<Point3D>& uniquePointsVec, std::unordered_set<Element>& UniqueElements, vector<Element>& NewUniqueElements){

    // I have to substitute the locally defined index of points to the global ones.

    uniquePointsVec.resize(DummyMapPoints.size());
    NewUniqueElements.resize(UniqueElements.size());
    std::unordered_set<Point3D> uniquePoints;
    // Dummy point map for points to be re-inserted
    long int totalIndex = 1;
    auto it = UniqueElements.begin();
    // cycle on every element
    cout << totalIndex << endl;
    while (it != UniqueElements.end()){
        
        // extract list of points
        vector<cgsize_t> Points = it->indexPoints;

        for(int i = 0; i < Points.size(); i++){
            // Define an accessory point
            Point3DExt pointExtHere(it->zoneIndex, Points[i]);
            // PointHere.print();

            // Now search for this point among the map of whole points
            auto findPoint = DummyMapPoints.find(pointExtHere);

            // Now search for this point among the map of unique points
            Point3D pointHere(*findPoint);
            auto findDup = uniquePoints.find(pointHere);

            // cout << std::distance(DummyMapPoints.begin(), findDup) << "  " << DummyMapPoints.size() << endl;

            // if(findDup == DummyMapPoints.end()) {
            //     cout << "wtf" << endl;
            // }
            // The first check should not be performed, while the second one should be.
            if (findDup == uniquePoints.end()) {
                // In this case, substitute the local index with the global one
                Points[i] = totalIndex;
                // If it has been found, then it has to be inserted among the unique points
                pointHere.totalIndex = totalIndex;
                uniquePoints.insert(pointHere);
                uniquePointsVec[totalIndex-1] = pointHere;
                totalIndex++;

            } else {
                // This should always happen only if a duplicated node is found
                // In this case, substitute the local index with the global one
                Points[i] = (cgsize_t)findDup->totalIndex;
            }

        }

        // Now save the changes
        Element dummyElement = *it;
        dummyElement.indexPoints = Points;
        NewUniqueElements[dummyElement.totalIndex] = dummyElement;
        // cout << totalIndex << endl;

        ++it;
    }

    uniquePointsVec.resize(totalIndex-1);

    cout << "Total number of points was "  << DummyMapPoints.size() << endl;
    cout << "Non-duplicated number of nodes is " << uniquePointsVec.size() << endl;

}


void print_stack_trace() {
    void* array[10];
    size_t size;

    // Get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // Print out all the frames to stderr
    backtrace_symbols_fd(array, size, STDERR_FILENO);
}