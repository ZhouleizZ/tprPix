/*
 * ======================= YardJson.cpp =======================
 *                          -- tpr --
 *                                        CREATE -- 2019.12.04
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 * handle yard-blueprint json files
 */
#include "YardBlueprint.h"


//--------------- Libs ------------------//
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "tprGeneral.h"

//--------------- Engine ------------------//
#include "global.h"
#include "fileIO.h"
#include "tprCast.h"
#include "blueprint_oth.h"

#include "esrc_state.h"

//--------------- Script ------------------//
#include "Script/json/json_oth.h" // tmp
#include "Script/resource/ssrc_gameObj.h" // tmp



using namespace rapidjson;

#include <iostream>
using std::cout;
using std::endl;


namespace blueprint {//------------------ namespace: blueprint start ---------------------//
namespace yardJson_inn {//-------- namespace: yardJson_inn --------------//

    void parse_from_single_yardJsonFile( const std::string &path_file_ );

}//------------- namespace: yardJson_inn end --------------//


void parse_from_yardJsonFiles(){

    cout << "   ----- parse_from_yardJsonFiles: start ----- " << endl;


    std::vector<std::string> path_files {};
    json::collect_fileNames( path_blueprintDatas, "yards", "_files.json", path_files );
    //---
    for( const auto &i : path_files ){
        yardJson_inn::parse_from_single_yardJsonFile(i);
    }

    cout << "   ----- parse_from_yardJsonFiles: end ----- " << endl;
}




namespace yardJson_inn {//-------- namespace: yardJson_inn --------------//


void parse_from_single_yardJsonFile( const std::string &path_file_ ){
    //-----------------------------//
    //         load file
    //-----------------------------//
    auto jsonBufUPtr = read_a_file( path_file_ );

    //-----------------------------//
    //      parce JSON data
    //-----------------------------//
    Document doc;
    doc.Parse( jsonBufUPtr->c_str() );

    std::string yardName {};
    std::string pngPath_M {};
    IntVec2 frameNum {};
    size_t  totalFrameNum {};

    bool isHaveMajorGos {};
    bool isHaveFloorGos {};
    

    //---
    tprAssert( doc.IsObject() );
    {//--- yardName ---//
        const auto &a = json::check_and_get_value( doc, "yardName", json::JsonValType::String );
        yardName = a.GetString();
    }

    //--- yard ---
    auto yardId = YardBlueprint::init_new_yard( yardName );
    auto &yardRef = YardBlueprint::get_yardBlueprintRef( yardId );



    {//--- pngLPath ---//
        const auto &a = json::check_and_get_value( doc, "pngLPath", json::JsonValType::String );
        std::string pngLPath = a.GetString();
        std::string headPath = tprGeneral::path_combine(path_blueprintDatas, "yards");
        pngPath_M = tprGeneral::path_combine( headPath, pngLPath );
    }
    {//--- frameNum.col ---//
        const auto &a = json::check_and_get_value( doc, "frameNum.col", json::JsonValType::Int );
        frameNum.x =  a.GetInt();
    }
    {//--- frameNum.row ---//
        const auto &a = json::check_and_get_value( doc, "frameNum.row", json::JsonValType::Int );
        frameNum.y =  a.GetInt();
    }
    {//--- totalFrameNum ---//
        const auto &a = json::check_and_get_value( doc, "totalFrameNum", json::JsonValType::Uint64 );
        totalFrameNum =  cast_2_size_t(a.GetUint64());
        tprAssert( totalFrameNum <= cast_2_size_t(frameNum.x * frameNum.y) );
    }
    {//--- isHaveMajorGos ---//
        const auto &a = json::check_and_get_value( doc, "isHaveMajorGos", json::JsonValType::Bool );
        isHaveMajorGos = a.GetBool();
    }
    {//--- isHaveFloorGos ---//
        const auto &a = json::check_and_get_value( doc, "isHaveFloorGos", json::JsonValType::Bool );
        isHaveFloorGos = a.GetBool();
    }

    //-- 至少一个值 为 true --
    tprAssert( isHaveMajorGos || isHaveFloorGos );
    yardRef.set_isHaveMajorGos( isHaveMajorGos );
    yardRef.set_isHaveFloorGos( isHaveFloorGos );
    

    // 读取解析 png 数据，
    IntVec2 frameSizeByMapEnt = parse_png_for_yard( yardRef, pngPath_M, frameNum, totalFrameNum );
    yardRef.set_sizeByField( frameSizeByMapEnt );


    VariableTypeIdx varTypeIdx {}; 
    bool isAllInstanceUseSamePlan {};
    bool isFloorGo {};

    //--- varTypes ---//
    const auto &varTypes = json::check_and_get_value( doc, "varTypes", json::JsonValType::Array );
    for( auto &varType : varTypes.GetArray() ){
        tprAssert( varType.IsObject() );

        {//--- type ---//
            const auto &a = json::check_and_get_value( varType, "type", json::JsonValType::String );
            varTypeIdx = str_2_variableTypeIdx( a.GetString() );
        }
        {//--- isAllInstanceUseSamePlan ---//
            const auto &a = json::check_and_get_value( varType, "isAllInstanceUseSamePlan", json::JsonValType::Bool );
            isAllInstanceUseSamePlan = a.GetBool();
        }
        {//--- isFloorGo ---//
            const auto &a = json::check_and_get_value( varType, "isFloorGo", json::JsonValType::Bool );
            isFloorGo = a.GetBool();
        }

        if( !isFloorGo ){
            //----------------//
            //    FloorGo
            //----------------//
            std::unique_ptr<VarTypeDatas_Yard_MajorGo> varTypeDatas_majorGoUPtr = std::make_unique<VarTypeDatas_Yard_MajorGo>();
            varTypeDatas_majorGoUPtr->set_isAllInstanceUseSamePlan( isAllInstanceUseSamePlan );

            {//--- isPlotBlueprint ---//
                const auto &a = json::check_and_get_value( varType, "isPlotBlueprint", json::JsonValType::Bool );
                varTypeDatas_majorGoUPtr->set_isPlotBlueprint( a.GetBool() );
            }

            if( varTypeDatas_majorGoUPtr->get_isPlotBlueprint() ){
                //--- plotNames ---//
                const auto &plotNames = json::check_and_get_value( varType, "plotNames", json::JsonValType::Array );
                for( auto &ent : plotNames.GetArray() ){
                    tprAssert( ent.IsString() );
                    varTypeDatas_majorGoUPtr->insert_2_plotIds( PlotBlueprint::str_2_plotBlueprintId(ent.GetString()) );
                }

            }else{
                //--- goSpecPool ---//
                const auto &goSpecPool = json::check_and_get_value( varType, "goSpecPool", json::JsonValType::Array );
                for( auto &ent : goSpecPool.GetArray() ){

                    std::unique_ptr<GoSpec> goSpecUPtr = std::make_unique<GoSpec>();

                    {//--- goSpecName ---//
                        const auto &a = json::check_and_get_value( ent, "goSpecName", json::JsonValType::String );
                        goSpecUPtr->goSpecId = ssrc::str_2_goSpecId( a.GetString() );
                    }
                    {//--- animLabels ---//
                        const auto &a = json::check_and_get_value( ent, "animLabels", json::JsonValType::Array );
                        if( a.Size() > 0 ){
                            for( auto &label : a.GetArray() ){//- foreach AnimLabel
                                tprAssert( label.IsString() );
                                goSpecUPtr->animLabels.push_back( str_2_AnimLabel(label.GetString()) );
                            }
                        }
                    }
                    //-- goSpecUPtr 创建完毕 --
                    varTypeDatas_majorGoUPtr->insert_2_goSpecPool( std::move(goSpecUPtr) );
                }
            }

            varTypeDatas_majorGoUPtr->init_check();
            yardRef.insert_2_majorGo_varTypeDatas( varTypeIdx, std::move(varTypeDatas_majorGoUPtr) );

        }else{
            //----------------//
            //    MajoeGo
            //----------------//
            std::unique_ptr<VarTypeDatas_Yard_FloorGo> varTypeDatas_floorGoUPtr = std::make_unique<VarTypeDatas_Yard_FloorGo>();
            varTypeDatas_floorGoUPtr->set_isAllInstanceUseSamePlan( isAllInstanceUseSamePlan );

            {//--- FloorGoSize ---//
                const auto &a = json::check_and_get_value( varType, "FloorGoSize", json::JsonValType::String );
                varTypeDatas_floorGoUPtr->set_floorGoSize( str_2_FloorGoSize(a.GetString()) );
            }

            {//--- goSpecPool ---//
                const auto &goSpecPool = json::check_and_get_value( varType, "goSpecPool", json::JsonValType::Array );
                for( auto &ent : goSpecPool.GetArray() ){

                    std::unique_ptr<GoSpec> goSpecUPtr = std::make_unique<GoSpec>();

                    {//--- goSpecName ---//
                        const auto &a = json::check_and_get_value( ent, "goSpecName", json::JsonValType::String );
                        goSpecUPtr->goSpecId = ssrc::str_2_goSpecId( a.GetString() );
                    }
                    {//--- animLabels ---//
                        const auto &a = json::check_and_get_value( ent, "animLabels", json::JsonValType::Array );
                        if( a.Size() > 0 ){
                            for( auto &label : a.GetArray() ){//- foreach AnimLabel
                                tprAssert( label.IsString() );
                                goSpecUPtr->animLabels.push_back( str_2_AnimLabel(label.GetString()) );
                            }
                        }
                    }
                    //-- goSpecUPtr 创建完毕 --
                    varTypeDatas_floorGoUPtr->insert_2_goSpecPool( std::move(goSpecUPtr) );
                }
            }
            varTypeDatas_floorGoUPtr->init_check();
            yardRef.insert_2_floorGo_varTypeDatas( varTypeIdx, std::move(varTypeDatas_floorGoUPtr) );

        }

    }

    yardRef.init_check();
}



}//------------- namespace: yardJson_inn end --------------//
}//--------------------- namespace: blueprint end ------------------------//
