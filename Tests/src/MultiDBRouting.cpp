/*
  MultiDBRouting - a test program for libosmscout
  Copyright (C) 2016  Tim Teulings

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#include <osmscout/Database.h>
#include <osmscout/Pixel.h>
#include <osmscout/routing/RoutingService.h>
#include <osmscout/routing/MultiDBRoutingService.h>

#include <osmscout/util/CmdLineParsing.h>
#include <osmscout/util/FileScanner.h>

struct Arguments
{
  bool                   help=false;
  std::string            databaseDirectory1;
  std::string            databaseDirectory2;
  osmscout::GeoCoord     start;
  osmscout::GeoCoord     target;
};

void GetCarSpeedTable(std::map<std::string,double>& map)
{
  map["highway_motorway"]=110.0;
  map["highway_motorway_trunk"]=100.0;
  map["highway_motorway_primary"]=70.0;
  map["highway_motorway_link"]=60.0;
  map["highway_motorway_junction"]=60.0;
  map["highway_trunk"]=100.0;
  map["highway_trunk_link"]=60.0;
  map["highway_primary"]=70.0;
  map["highway_primary_link"]=60.0;
  map["highway_secondary"]=60.0;
  map["highway_secondary_link"]=50.0;
  map["highway_tertiary"]=55.0;
  map["highway_tertiary_link"]=55.0;
  map["highway_unclassified"]=50.0;
  map["highway_road"]=50.0;
  map["highway_residential"]=40.0;
  map["highway_roundabout"]=40.0;
  map["highway_living_street"]=10.0;
  map["highway_service"]=30.0;
}

int main(int argc, char* argv[])
{
  osmscout::CmdLineParser   argParser("MutiDBRouting",
                                      argc,argv);
  std::vector<std::string>  helpArgs{"h","help"};
  Arguments                 args;

  argParser.AddOption(osmscout::CmdLineFlag([&args](const bool& value) {
                        args.help=value;
                      }),
                      helpArgs,
                      "Return argument help",
                      true);

  argParser.AddPositional(osmscout::CmdLineStringOption([&args](const std::string& value) {
                            args.databaseDirectory1=value;
                          }),
                          "DATABASE1",
                          "Directory of the first database to use");

  argParser.AddPositional(osmscout::CmdLineStringOption([&args](const std::string& value) {
                            args.databaseDirectory2=value;
                          }),
                          "DATABASE2",
                          "Directory of the second database to use");

  argParser.AddPositional(osmscout::CmdLineGeoCoordOption([&args](const osmscout::GeoCoord& value) {
                            args.start=value;
                          }),
                          "START",
                          "start coordinate");

  argParser.AddPositional(osmscout::CmdLineGeoCoordOption([&args](const osmscout::GeoCoord& value) {
                            args.target=value;
                          }),
                          "TARGET",
                          "target coordinate");

  osmscout::CmdLineParseResult result=argParser.Parse();

  if (result.HasError()) {
    std::cerr << "ERROR: " << result.GetErrorDescription() << std::endl;
    std::cout << argParser.GetHelp() << std::endl;
    return 1;
  }

  if (args.help) {
    std::cout << argParser.GetHelp() << std::endl;
    return 0;
  }

  // Database

  osmscout::DatabaseParameter dbParameter;
  osmscout::DatabaseRef       database1=std::make_shared<osmscout::Database>(dbParameter);
  osmscout::DatabaseRef       database2=std::make_shared<osmscout::Database>(dbParameter);
  osmscout::RouterParameter   routerParameter;

  osmscout::log.Debug(true);
  osmscout::log.Info(true);
  osmscout::log.Warn(true);
  osmscout::log.Error(true);

  routerParameter.SetDebugPerformance(true);

  std::cout << "Opening database 1..." << std::endl;

  if (!database1->Open(args.databaseDirectory1)) {
    std::cerr << "Cannot open database 1" << std::endl;

    return 1;
  }

  std::cout << "Done." << std::endl;

  std::cout << "Opening database 2..." << std::endl;

  if (!database2->Open(args.databaseDirectory2)) {
    std::cerr << "Cannot open database 2" << std::endl;

    return 1;
  }

  std::cout << "Done." << std::endl;

  std::vector<osmscout::DatabaseRef> databases={database1,database2};

  osmscout::RouterParameter routerParam;
  routerParam.SetDebugPerformance(true);
  osmscout::MultiDBRoutingServiceRef router=std::make_shared<osmscout::MultiDBRoutingService>(routerParam,databases);

  std::cout << "Opening router..." << std::endl;

  osmscout::MultiDBRoutingService::RoutingProfileBuilder profileBuilder=
      [](const osmscout::DatabaseRef &database){
        auto profile=std::make_shared<osmscout::FastestPathRoutingProfile>(database->GetTypeConfig());
        std::map<std::string,double> speedMap;
        GetCarSpeedTable(speedMap);
        profile->ParametrizeForCar(*(database->GetTypeConfig()),speedMap,160.0);
        return profile;
      };

  if (!router->Open(profileBuilder)) {

    std::cerr << "Cannot open router" << std::endl;
    return 1;
  }
  std::cout << "Done." << std::endl;

  osmscout::RoutePosition startNode=router->GetClosestRoutableNode(args.start);
  if (!startNode.IsValid()){
    std::cerr << "Can't found route node near start coord " << args.start.GetDisplayText() << std::endl;
    return 1;
  }
  osmscout::RoutePosition targetNode=router->GetClosestRoutableNode(args.target);
  if (!targetNode.IsValid()){
    std::cerr << "Can't found route node near target coord " << args.target.GetDisplayText() << std::endl;
    return 1;
  }

  osmscout::RoutingParameter parameter;
  osmscout::RoutingResult route=router->CalculateRoute(startNode,targetNode,parameter);
  if (!route.Success()){
    std::cerr << "Route failed" << std::endl;
    return 1;
  }

  osmscout::RouteDescription description;
  router->TransformRouteDataToRouteDescription(route.GetRoute(),description);

  std::cout << "Closing RoutingServices and databases..." << std::endl;

  router->Close();

  database1->Close();
  database1.reset();

  database2->Close();
  database2.reset();

  std::cout << "Done." << std::endl;

  return 0;
}
