//
// Created by rausa on 20/12/22.
//



#include "functions.h"

#if CGNS_VERSION < 3100
# define cgsize_t int
#endif





int main (int args, char **input){

    int index_file,index_zone;
    int nsections,index_sect;
    char bcname[33];
    // string file2open = "/home/rausa/Software/lib/CGNSMod/Meshes/provaTMore.cgns";
    // string file2open = "/home/rausa/Software/lib/CGNSMod/Meshes/provaT.cgns";
    // string file2open = "/home/rausa/Software/lib/CGNSMod/Meshes/provaTria_ConQuads.cgns";
    string file2open = "/home/rausa/Software/lib/CGNSMod/Meshes/CoarseLoro.cgns";


/* READ X, Y, Z GRID POINTS FROM CGNS FILE */
/* open CGNS file for read-only */
    // if (cg_open("/home/rausa/Software/lib/CGNSMod/Meshes/CoarseLoro.cgns",CG_MODE_READ,&index_file)) cg_error_exit();
    if (cg_open(file2open.c_str(),CG_MODE_READ,&index_file)) cg_error_exit();
    // if (cg_open("/home/rausa/Software/lib/CGNSMod/Meshes/provaTMore.cgns",CG_MODE_READ,&index_file)) cg_error_exit();

    int nbases, nzones, nbcs;

    if (cg_nbases(index_file, &nbases)) cg_error_exit();
    
    printf("numero di basi: %d\n", nbases);
    
    if (cg_nzones(index_file, 1, &nzones)) cg_error_exit();
 
    printf("numero di zone: %d\n", nzones); 

    if(nzones > 1){
        
        vector<vector<vector<cgsize_t>>> BCElements(nzones, vector<vector<cgsize_t>>());
        vector<vector<string>> BCNames(nzones, vector<string>());

        for (index_zone = 1; index_zone <= nzones; index_zone++)
        {
            printf("\nReading zone %d data...\n", index_zone);

            cg_nbocos(index_file, 1, index_zone, &nbcs);

            printf("   This zone has %d boundary conditions\n", nbcs);

            BCNames[index_zone-1].resize(nbcs);
            BCElements[index_zone-1].resize(nbcs);

            for( int index_bc = 1; index_bc <= nbcs; index_bc++) {

                BCType_t bctype;
                PointSetType_t bc_pttype;
                cgsize_t nBCElementsInfo;
                int NormalIndex;
                cgsize_t NormalListSize;
                DataType_t NormalDataType;
                int ndataSet;
                GridLocation_t boh;

                cg_boco_info(index_file, 1, index_zone, index_bc, bcname, &bctype, &bc_pttype, &nBCElementsInfo, &NormalIndex, &NormalListSize, &NormalDataType, &ndataSet);
                cg_boco_gridlocation_read(index_file, 1, index_zone, index_bc, &boh);
                // printf("  %d   %d    ", bctype, boh);
                BCNames[index_zone-1][index_bc-1] = bcname;


                vector<cgsize_t> BCElementsInfo((int)nBCElementsInfo);
                cg_boco_read(index_file, 1, index_zone, index_bc, BCElementsInfo.data(), NULL);
                
                int nElements;

                switch (bc_pttype) {
                    case PointList:
                        cout << "it is a point list... ";
                        nElements = nBCElementsInfo;
                        BCElements[index_zone-1][index_bc-1].resize(nElements);
                        BCElements[index_zone-1][index_bc-1] = BCElementsInfo;
                        printf("   bc name is %s and it has %d elements, for sure between from %d to %d\n",bcname, nElements, BCElementsInfo[0], BCElementsInfo[0]+nElements-1);
                        break;
                    case PointRange:
                        cout << "it is a point range... ";
                        nElements = (int)BCElementsInfo[1] - (int)BCElementsInfo[0] + 1;
                        BCElements[index_zone-1][index_bc-1].resize(nElements);
                        iota(BCElements[index_zone-1][index_bc-1].begin(), BCElements[index_zone-1][index_bc-1].end(), BCElementsInfo[0]);  // L'indice dei punti inizia da 1
                        printf("   bc name is %s and it has %d elements, from %d to %d\n",bcname, nElements, BCElementsInfo[0], BCElementsInfo[0]+nElements-1);
                        break;
                    default:
                        printf("Such point set (%d) has not been considered. Exiting...", bc_pttype);
                        return 1;
                }

                // printf(" %d, %d, %d\n", BCElementsInfo[1], BCPoints[index_zone-1][0], BCPoints[index_zone-1][BCPoints[index_zone-1].size()-1]);
            }

        }

        // Now check for boundary conditions with the same name across two zones
        bool isThereACommonBoundary = false;
        vector<pair<string, pair<int, int>>> CommonBoundaries;
        vector<string> CommonBoundariesOnly;
        vector<pair<int, int>> AbuttingZones;
        std::unordered_set<Element> Elements2Remove;
        int NElements2Remove = 1;
        for (index_zone = 1; index_zone <= nzones-1; index_zone++) {
            
            vector<string> ThisBoundaries = BCNames[index_zone-1];

            for (int index_zone_2 = index_zone+1; index_zone_2 <= nzones; index_zone_2++) {
                
                for( int index_bc = 1; index_bc <= BCNames[index_zone_2-1].size(); index_bc++) {

                    auto it = find(ThisBoundaries.begin(), ThisBoundaries.end(), BCNames[index_zone_2-1][index_bc-1]);

                    // Check if the target string was found
                    if (it != ThisBoundaries.end()) {
                        
                        AbuttingZones.push_back(make_pair(index_zone, index_zone_2));
                        CommonBoundaries.push_back(make_pair(BCNames[index_zone_2-1][index_bc-1], make_pair(std::distance(ThisBoundaries.begin(), it)+1, index_bc)));
                        CommonBoundariesOnly.push_back(BCNames[index_zone_2-1][index_bc-1]);
                        cout << CommonBoundaries[CommonBoundaries.size()-1].first << " " << CommonBoundaries[CommonBoundaries.size()-1].second.first <<  " " << CommonBoundaries[CommonBoundaries.size()-1].second.second << endl;
                        isThereACommonBoundary= true;

                        // Update the structure for the elements to remove
                        // First zone_index_2
                        for (size_t iElement = 0; iElement < BCElements[index_zone_2-1][index_bc-1].size(); iElement++){
                            Element Element2Remove;
                            Element2Remove.totalIndex = NElements2Remove;
                            NElements2Remove++;
                            Element2Remove.zoneIndex = index_zone_2;
                            Element2Remove.localLocalIndex = BCElements[index_zone_2-1][index_bc-1][iElement];
                            // cout << "zone = " << index_zone_2 << " " << Element2Remove.zoneIndex << " " << Element2Remove.localIndex << endl;
                            Elements2Remove.insert(Element2Remove);
                        }
                        // Then zone_index_1
                        for (size_t iElement = 0; iElement < BCElements[index_zone-1][std::distance(ThisBoundaries.begin(), it)].size(); iElement++){
                            Element Element2Remove;
                            Element2Remove.totalIndex = NElements2Remove;
                            NElements2Remove++;
                            Element2Remove.zoneIndex = index_zone;
                            Element2Remove.localLocalIndex = BCElements[index_zone-1][std::distance(ThisBoundaries.begin(), it)][iElement];
                            // cout << "zone = " << index_zone << " " << Element2Remove.zoneIndex << " " << Element2Remove.localIndex << endl;
                            Elements2Remove.insert(Element2Remove);
                        }
                    } 
                }
            }
        }

        if (isThereACommonBoundary) {
            for (int i = 0; i < CommonBoundaries.size(); i++){
                cout << "Zone " << AbuttingZones[i].first << " and zone " << AbuttingZones[i].second << " have in common the boundary: ";
                cout << CommonBoundaries[i].first << endl;
            }
            cout << endl;
        }

        // Now extract the boundaries that will be kept
        vector<string> boundaries2Keep;
        if (isThereACommonBoundary) {
            cout << "Boundaries that will be kept are: " << endl;
            for (index_zone = 1; index_zone <= nzones; index_zone++) {
                vector<string> ThisBoundaries = BCNames[index_zone-1];
                for (int i = 0; i < ThisBoundaries.size(); i++){
                    auto it = find(CommonBoundariesOnly.begin(), CommonBoundariesOnly.end(), ThisBoundaries[i]);
                    if( it == CommonBoundariesOnly.end()){
                        boundaries2Keep.push_back(ThisBoundaries[i]);
                        cout << ThisBoundaries[i] << " from zone " << index_zone << endl;
                    }
                }

            }

        }

        std::unordered_set<Point3D> coordinates;
        std::map<pair<int, cgsize_t>, Element> Elements;

        vector<vector<long int>> PointDict;
        std::unordered_set<Point3DExt> DummyMapPoints;
        if(extractPointsAndElements(file2open, coordinates, PointDict, Elements, DummyMapPoints) == 1) return 1;

        cout << "Done... " << endl; 

        // for(size_t i = 0; i < Elements.size(); i++){
        //     cout << "Element " << Elements[i].totalIndex << " belongs to zone " << Elements[i].zoneIndex;
        //     cout << " has type " << Elements[i].type << " and local local index ";
        //     cout << Elements[i].localLocalIndex << endl;
        // }
        
        // cout << endl << endl << endl << endl;

        // for(auto it = Elements2Remove.begin(); it != Elements2Remove.end(); it++){
        //     cout << "Element " << it->totalIndex << " belongs to zone " << it->zoneIndex;
        //     cout << " has index points = [" << endl;
        // }

        // Remove elements if they are from a duplicated boundary condition
        std::map<pair<int, cgsize_t>, Element> UniqueElements = Elements;
        // for(size_t i = 0; i < UniqueElements.size(); i++){
        //     cout << "Element " << UniqueElements[i].totalIndex << " belongs to zone " << UniqueElements[i].zoneIndex;
        //     cout << " has type " << UniqueElements[i].type << " and local local index ";
        //     cout << UniqueElements[i].localLocalIndex << endl;
        // }
        removeAbuttingElements(Elements2Remove, UniqueElements);


        // Create a map to store the removed indices and corresponding original indices
        std::unordered_map<size_t, size_t> removedIndices;

        // Remove duplicate coordinates
        // removeDuplicates(coordinates, removedIndices, uniqueCoordinates, globalUniquePoints, DummyMapPoints);
        // PROBLEMMMM!!!!!!!!!!
        // RemoveDucplicates also removes the points at the boundary of the overlapping zones. I should first remove the elements,
        // then remove all the duplicate points but those belonging to the boundary. How to do it??
        // OOOOOOOORRR I can re-introduce those points that do not find any correspondence since they will for sure be associated with the boundaries
        // of the overlapping regions!!!

        // for(auto it = DummyMapPoints.begin(); it != DummyMapPoints.end(); it++){
        //     auto cose = *it;
        //     cose.print();
        // }


        // Now we just need to assign correct indexes of the points of the elements 
        vector<Element> NewUniqueElements;
        std::unordered_set<Point3D> uniqueCoordinates;
        vector<Point3D> uniquePointsVec;
        assignNewPointIndices(DummyMapPoints, uniquePointsVec, UniqueElements, NewUniqueElements);

        // for (auto it = removedIndices.begin(); it != removedIndices.end(); it++) {
        //     printf("Point %d (%.5f %.5f %.5f) is equal to point %d (%.5f %.5f %.5f)\n", it->first,
        //              coordinates[it->first].x, coordinates[it->first].y, coordinates[it->first].z, it->second,
        //              coordinates[it->second].x, coordinates[it->second].y, coordinates[it->second].z);
        // }

        cg_close(index_file);

        string file2write = file2open.substr(0, file2open.size()-5) + "_SingleZone.cgns";
        cout << file2write << endl;
        cg_open(file2write.c_str(), CG_MODE_WRITE, &index_file);


        // Now I have to assign the elements such that they are divided by element type
        // I will declare them with the largest possible size and the nresize accordingly
        vector<vector<cgsize_t>> triaElements(NewUniqueElements.size());
        vector<vector<cgsize_t>> quadElements(NewUniqueElements.size());
        vector<vector<cgsize_t>> tetraElements(NewUniqueElements.size());
        vector<vector<cgsize_t>> pyraElements(NewUniqueElements.size());
        vector<vector<cgsize_t>> pentaElements(NewUniqueElements.size());
        vector<vector<cgsize_t>> hexaElements(NewUniqueElements.size());

        // Sort the Element vector. Now I should have them sorted and easier to access.
        // sort(NewUniqueElements.begin(), NewUniqueElements.end(), Element::sortFun);

        cgsize_t nTria, nQuad, nTetra, nPyra, nPenta, nHexa; 
        nTria = nQuad = nTetra = nPyra = nPenta = nHexa = 0;

        for (cgsize_t i = 0; i < NewUniqueElements.size(); i++) {
            switch (NewUniqueElements[i].type) {
                case TRI_3: 
                    triaElements[nTria] = NewUniqueElements[i].indexPoints;
                    NewUniqueElements[i].actualElementNumber = nTria;
                    nTria++;
                    break;
                case QUAD_4:
                    quadElements[nQuad] = NewUniqueElements[i].indexPoints;
                    NewUniqueElements[i].actualElementNumber = nQuad;
                    nQuad++;
                    break;
                case TETRA_4:
                    tetraElements[nTetra] = NewUniqueElements[i].indexPoints;
                    NewUniqueElements[i].actualElementNumber = nTetra;
                    nTetra++;
                    break;
                case PYRA_5: 
                    pyraElements[nPyra] = NewUniqueElements[i].indexPoints;
                    NewUniqueElements[i].actualElementNumber = nPyra;
                    nPyra++;
                    break;
                case PENTA_6:
                    pentaElements[nPenta] = NewUniqueElements[i].indexPoints;
                    NewUniqueElements[i].actualElementNumber = nPenta;
                    nPenta++;
                    break;
                case HEXA_8:
                    hexaElements[nHexa] = NewUniqueElements[i].indexPoints;
                    NewUniqueElements[i].actualElementNumber = nHexa;
                    nHexa++;
                    break;
                default: 
                    printf("Elemento %s non riconosciuto. Exiting..", ElementTypeName[NewUniqueElements[i].type]);
                    return 1;
            }
        }

        // Now save the true actual index
        // Even if starting from a not sorted, it should work just fine
        for (cgsize_t i = 0; i < NewUniqueElements.size(); i++) {
            switch (NewUniqueElements[i].type) {
                case TRI_3: 
                    // Trias are the first ones thus, it is good
                    break;
                case QUAD_4:
                    NewUniqueElements[i].actualElementNumber = nTria+
                                                               NewUniqueElements[i].actualElementNumber;
                    break;
                case TETRA_4:
                    NewUniqueElements[i].actualElementNumber = nTria + nQuad +
                                                               NewUniqueElements[i].actualElementNumber;
                    break;
                case PYRA_5: 
                    NewUniqueElements[i].actualElementNumber = nTria + nQuad + nTetra + 
                                                               NewUniqueElements[i].actualElementNumber;
                    break;
                case PENTA_6:
                    NewUniqueElements[i].actualElementNumber = nTria + nQuad + nTetra + nPyra +
                                                               NewUniqueElements[i].actualElementNumber;
                    break;
                case HEXA_8:
                    NewUniqueElements[i].actualElementNumber = nTria + nQuad + nTetra + nPyra + nPenta +
                                                               NewUniqueElements[i].actualElementNumber;
                    break;
                default: 
                    printf("Elemento %s non riconosciuto. Exiting..", ElementTypeName[NewUniqueElements[i].type]);
                    return 1;
            }
            // cout   << ElementTypeName[NewUniqueElements[i].type] << " has number " << NewUniqueElements[i].actualElementNumber +1 << endl;
        }


        // Now resize to actual size
        triaElements.resize(nTria);
        quadElements.resize(nQuad);
        tetraElements.resize(nTetra);
        pyraElements.resize(nPyra);
        pentaElements.resize(nPenta);
        hexaElements.resize(nHexa);
 
        cout << "Total number of elements divided by type:" << endl;
        cout << "Tria elements = " << triaElements.size() << endl;
        cout << "Quad elements = " << quadElements.size() << endl;
        cout << "Tetra elements = " << tetraElements.size() << endl;
        cout << "Pyramid elements = " << pyraElements.size() << endl;
        cout << "Penta elements = " << pentaElements.size() << endl;
        cout << "Hexa elements = " << hexaElements.size() << endl;


        cgsize_t TotalSizes[3];
        TotalSizes[0] = uniquePointsVec.size();
        TotalSizes[1] = nTetra+nPyra+nPenta+nHexa;
        TotalSizes[2] = boundaries2Keep.size();
        int one;
        cg_base_write(index_file, "mesh", 3, 3, &one);
        cg_zone_write(index_file, 1, "Zone 1", TotalSizes, Unstructured, &one);

        int xid;
        vector<double> TotalXCoords(uniquePointsVec.size(), 0.0);
        vector<double> TotalYCoords(uniquePointsVec.size(), 0.0);
        vector<double> TotalZCoords(uniquePointsVec.size(), 0.0);
        for (long int i = 0; i < uniquePointsVec.size(); i++){
            TotalXCoords[i] = uniquePointsVec[i].x;
            TotalYCoords[i] = uniquePointsVec[i].y;
            TotalZCoords[i] = uniquePointsVec[i].z;
        }
        cg_coord_write(index_file, 1, 1, RealDouble, "CoordinateX", TotalXCoords.data(), &xid);
        cg_coord_write(index_file, 1, 1, RealDouble, "CoordinateY", TotalYCoords.data(), &xid);
        cg_coord_write(index_file, 1, 1, RealDouble, "CoordinateZ", TotalZCoords.data(), &xid);

        // Now write the different sections
        int triaSection, quadSection, tetraSection, pyraSection, pentaSection, hexaSection, lastSection;
        triaSection = quadSection = tetraSection = pyraSection = pentaSection = hexaSection = lastSection = 0;
        cgsize_t lastNofElements = 1;
        string sectionName;
        if(nTria != 0){
            triaSection = 1;
            lastSection = triaSection;
            sectionName = "Tria";
            int S;
            const int nPoints = 3;
            vector<cgsize_t> elem2Write(triaElements.size()*nPoints);
            cgsize_t n = 0;
            for (cgsize_t i = 0; i < triaElements.size(); i++){
                for (cgsize_t j = 0; j < nPoints; j++){
                    elem2Write[n++] = triaElements[i][j];
                }
            }
            cg_section_write(index_file, 1, 1, sectionName.c_str(), TRI_3, lastNofElements, lastNofElements+nTria-1, 0, elem2Write.data(), &S);
            cout << "Trias go from "  << lastNofElements << " to " << lastNofElements+nTria-1 << endl;
            lastNofElements += nTria;
        }

        if(nQuad != 0){
            quadSection = lastSection++;
            sectionName = "Quads";
            int S;
            const int nPoints = 4;
            vector<cgsize_t> elem2Write(quadElements.size()*nPoints);
            cgsize_t n = 0;
            for (cgsize_t i = 0; i < quadElements.size(); i++){
                for (cgsize_t j = 0; j < nPoints; j++){
                    elem2Write[n++] = quadElements[i][j];
                }
            }
            cg_section_write(index_file, 1, 1, sectionName.c_str(), QUAD_4, lastNofElements, lastNofElements+nQuad-1, 0, elem2Write.data(), &S);
            cout << "Quads go from "  << lastNofElements << " to " << lastNofElements+nQuad-1 << endl;
            lastNofElements += nQuad;
        }

        if(nTetra != 0){
            tetraSection = lastSection++;
            sectionName = "Tetra";
            int S;
            const int nPoints = 4;
            vector<cgsize_t> elem2Write(tetraElements.size()*nPoints);
            cgsize_t n = 0;
            for (cgsize_t i = 0; i < tetraElements.size(); i++){
                for (cgsize_t j = 0; j < nPoints; j++){
                    elem2Write[n++] = tetraElements[i][j];
                }
            }
            cg_section_write(index_file, 1, 1, sectionName.c_str(), TETRA_4, lastNofElements, lastNofElements+nTetra-1, 0, elem2Write.data(), &S);
            cout << "Tetra go from "  << lastNofElements << " to " << lastNofElements+nTetra-1 << endl;
            lastNofElements += nTetra;
        }

        if(nPyra != 0){
            pyraSection = lastSection++;
            sectionName = "Pyramids";
            int S;
            const int nPoints = 5;
            vector<cgsize_t> elem2Write(pyraElements.size()*nPoints);
            cgsize_t n = 0;
            for (cgsize_t i = 0; i < pyraElements.size(); i++){
                for (cgsize_t j = 0; j < nPoints; j++){
                    elem2Write[n++] = pyraElements[i][j];
                }
            }
            cg_section_write(index_file, 1, 1, sectionName.c_str(), PYRA_5, lastNofElements, lastNofElements+nPyra-1, 0, elem2Write.data(), &S);
            cout << "Pyra go from "  << lastNofElements << " to " << lastNofElements+nPyra-1 << endl;
            lastNofElements += nPyra;
        }

        if(nPenta != 0){
            pentaSection = lastSection++;
            sectionName = "Penta";
            int S;
            const int nPoints = 6;
            vector<cgsize_t> elem2Write(pentaElements.size()*nPoints);
            cgsize_t n = 0;
            for (cgsize_t i = 0; i < pentaElements.size(); i++){
                for (cgsize_t j = 0; j < nPoints; j++){
                    elem2Write[n++] = pentaElements[i][j];
                }
            }
            cg_section_write(index_file, 1, 1, sectionName.c_str(), PENTA_6, lastNofElements, lastNofElements+nPenta-1, 0, elem2Write.data(), &S);
            cout << "Penta go from "  << lastNofElements << " to " << lastNofElements+nPenta-1 << endl;
            lastNofElements += nPenta;
        }

        if(nHexa != 0){
            hexaSection = lastSection++;
            sectionName = "Hexa";
            int S;
            const int nPoints = 8;
            vector<cgsize_t> elem2Write(hexaElements.size()*nPoints);
            cgsize_t n = 0;
            for (cgsize_t i = 0; i < hexaElements.size(); i++){
                for (cgsize_t j = 0; j < nPoints; j++){
                    elem2Write[n++] = hexaElements[i][j];
                }
            }
            cg_section_write(index_file, 1, 1, sectionName.c_str(), HEXA_8, lastNofElements, lastNofElements+nHexa-1, 0, elem2Write.data(), &S);
            cout << "Hexa go from "  << lastNofElements << " to " << lastNofElements+nHexa-1 << endl;
            lastNofElements += nHexa;
        }


        // It is much faster if I use a map
        std::map<pair<int, cgsize_t>, Element> elementMap;
        for (cgsize_t iElem = 0; iElem < NewUniqueElements.size(); iElem++){
            elementMap[{NewUniqueElements[iElem].zoneIndex, NewUniqueElements[iElem].localLocalIndex}] = NewUniqueElements[iElem];
        }

        // Now I should assign the correct boundary elements to the boundaries
        if (isThereACommonBoundary) {
            for (index_zone = 1; index_zone <= nzones; index_zone++) {
                vector<string> ThisBoundaries = BCNames[index_zone-1];
                for (int i = 0; i < ThisBoundaries.size(); i++){
                    auto it = find(CommonBoundariesOnly.begin(), CommonBoundariesOnly.end(), ThisBoundaries[i]);
                    if( it == CommonBoundariesOnly.end()){

                        // I have to modify the elements of this boundary and also write to file
                        vector<cgsize_t> elemHere = BCElements[index_zone-1][i];
                    
                        cout << "Writing boundary condition " << ThisBoundaries[i] << " with " << elemHere.size() << " elements" << endl;

                        for (cgsize_t iElem = 0; iElem < elemHere.size(); iElem++)
                        {
                            cgsize_t toFind = elemHere[iElem];
                            auto it = elementMap.find(make_pair(index_zone, toFind));

                            if (it != elementMap.end()) {
                                // It should always be found
                                auto cose = it->second;
                                elemHere[iElem] = cose.actualElementNumber+1;
                            } else {
                                std::cout << "Element not found\n";
                            }
                        }

                        int index_bc;
                        cg_boco_write(index_file, 1, 1, ThisBoundaries[i].c_str(), BCTypeUserDefined, ElementList, elemHere.size(), elemHere.data(), &index_bc);

                        
                    }
                }

            }

        }



        cg_close(index_file);

        return 0;

    }

  return 0;

}