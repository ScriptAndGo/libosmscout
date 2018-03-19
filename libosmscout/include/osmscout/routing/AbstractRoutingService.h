#ifndef OSMSCOUT_ABSTRACTROUTINGSERVICE_H
#define OSMSCOUT_ABSTRACTROUTINGSERVICE_H

/*
  This source is part of the libosmscout library
  Copyright (C) 2016  Tim Teulings
  Copyright (C) 2017  Lukas Karas

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#include <functional>
#include <list>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <osmscout/CoreFeatures.h>
#include <osmscout/TypeConfig.h>
#include <osmscout/Point.h>
#include <osmscout/Pixel.h>

#include <osmscout/routing/Route.h>
#include <osmscout/routing/RouteData.h>
#include <osmscout/routing/RouteNode.h>
#include <osmscout/routing/RoutingService.h>
#include <osmscout/routing/MultiDBRoutingState.h>

namespace osmscout {

  /**
   * \ingroup Routing
   *
   * Abstract algorithms for routing
   */
  template <class RoutingState>
  class OSMSCOUT_API AbstractRoutingService: public RoutingService
  {
  protected:
    bool debugPerformance;

  protected:
    virtual Vehicle GetVehicle(const RoutingState& state) = 0;

    virtual bool CanUse(const RoutingState& state,
                        DatabaseId database,
                        const RouteNode& routeNode,
                        size_t pathIndex) = 0;

    virtual bool CanUseForward(const RoutingState& state,
                               const DatabaseId& database,
                               const WayRef& way) = 0;

    virtual bool CanUseBackward(const RoutingState& state,
                                const DatabaseId& database,
                                const WayRef& way) = 0;

    virtual double GetCosts(const RoutingState& state,
                            DatabaseId database,
                            const RouteNode& routeNode,
                            size_t pathIndex) = 0;

    virtual double GetCosts(const RoutingState& state,
                            DatabaseId database,
                            const WayRef &way,
                            double wayLength) = 0;

    virtual double GetEstimateCosts(const RoutingState& state,
                                    DatabaseId database,
                                    double targetDistance) = 0;

    virtual double GetCostLimit(const RoutingState& state,
                                DatabaseId database,
                                double targetDistance) = 0;

    virtual bool GetRouteNode(const DatabaseId &database,
                              const Id &id,
                              RouteNodeRef &node) = 0;

    virtual bool GetRouteNodesByOffset(const std::set<DBFileOffset> &routeNodeOffsets,
                                       std::unordered_map<DBFileOffset,RouteNodeRef> &routeNodeMap) = 0;

    /**
     * Return the route node for the given database offset
     * @param offset
     *    Offset in given database
     * @param node
     *    Node instance to write the result back
     * @return
     *    True, if the node couldbe loaded, else false
     */
    virtual bool GetRouteNodeByOffset(const DBFileOffset &offset,
                                      RouteNodeRef &node) = 0;

    virtual bool GetWayByOffset(const DBFileOffset &offset,
                                WayRef &way) = 0;

    virtual bool GetWaysByOffset(const std::set<DBFileOffset> &wayOffsets,
                                 std::unordered_map<DBFileOffset,WayRef> &wayMap) = 0;

    virtual bool GetAreaByOffset(const DBFileOffset &offset,
                                 AreaRef &area) = 0;

    virtual bool GetAreasByOffset(const std::set<DBFileOffset> &areaOffsets,
                                  std::unordered_map<DBFileOffset,AreaRef> &areaMap) = 0;

    void ResolveRNodeChainToList(DBFileOffset finalRouteNode,
                                 const ClosedSet& closedSet,
                                 const ClosedSet &closedRestrictedSet,
                                 std::list<VNode>& nodes);

    virtual bool ResolveRouteDataJunctions(RouteData& route) = 0;

    virtual std::vector<DBFileOffset> GetNodeTwins(const RoutingState& state,
                                                   DatabaseId database,
                                                   Id id) = 0;

    void GetStartForwardRouteNode(const RoutingState& state,
                                  const DatabaseId& database,
                                  const WayRef& way,
                                  size_t nodeIndex,
                                  RouteNodeRef& routeNode,
                                  size_t& routeNodeIndex);
    void GetStartBackwardRouteNode(const RoutingState& state,
                                   const DatabaseId& database,
                                   const WayRef& way,
                                   size_t nodeIndex,
                                   RouteNodeRef& routeNode,
                                   size_t& routeNodeIndex);
    void GetTargetForwardRouteNode(const RoutingState& state,
                                   const DatabaseId& database,
                                   const WayRef& way,
                                   size_t nodeIndex,
                                   RouteNodeRef& routeNode);
    void GetTargetBackwardRouteNode(const RoutingState& state,
                                    const DatabaseId& database,
                                    const WayRef& way,
                                    size_t nodeIndex,
                                    RouteNodeRef& routeNode);

    bool GetStartNodes(const RoutingState& state,
                       const RoutePosition& position,
                       GeoCoord& startCoord,
                       const GeoCoord& targetCoord,
                       RouteNodeRef& forwardRouteNode,
                       RouteNodeRef& backwardRouteNode,
                       RNodeRef& forwardRNode,
                       RNodeRef& backwardRNode);

    bool GetWayTargetNodes(const RoutingState& state,
                           const RoutePosition& position,
                           GeoCoord& targetCoord,
                           RouteNodeRef& forwardNode,
                           RouteNodeRef& backwardNode);

    bool GetTargetNodes(const RoutingState& state,
                        const RoutePosition& position,
                        GeoCoord& targetCoord,
                        RouteNodeRef& forwardNode,
                        RouteNodeRef& backwardNode);

    bool GetRNode(const RoutingState& state,
                  const RoutePosition& position,
                  const WayRef& way,
                  size_t routeNodeIndex,
                  const RouteNodeRef& routeNode,
                  const GeoCoord& startCoord,
                  const GeoCoord& targetCoord,
                  RNodeRef& node);

    void AddNodes(RouteData& route,
                  DatabaseId database,
                  Id startNodeId,
                  size_t startNodeIndex,
                  const ObjectFileRef& object,
                  size_t idCount,
                  bool oneway,
                  size_t targetNodeIndex);

    bool GetWayStartNodes(const RoutingState& state,
                          const RoutePosition& position,
                          GeoCoord& startCoord,
                          const GeoCoord& targetCoord,
                          RouteNodeRef& forwardRouteNode,
                          RouteNodeRef& backwardRouteNode,
                          RNodeRef& forwardRNode,
                          RNodeRef& backwardRNode);

    bool ResolveRNodesToRouteData(const RoutingState& state,
                                  const std::list<VNode>& nodes,
                                  const RoutePosition& start,
                                  const RoutePosition& target,
                                  RouteData& route);

    virtual bool WalkToOtherDatabases(const RoutingState& state,
                                      RNodeRef &current,
                                      RouteNodeRef &currentRouteNode,
                                      OpenList &openList,
                                      OpenMap &openMap,
                                      const ClosedSet &closedSet,
                                      const ClosedSet &closedRestrictedSet);

    virtual bool WalkPaths(const RoutingState& state,
                           RNodeRef &current,
                           RouteNodeRef &currentRouteNode,
                           OpenList &openList,
                           OpenMap &openMap,
                           ClosedSet &closedSet,
                           ClosedSet &closedRestrictedSet,
                           RoutingResult &result,
                           const RoutingParameter& parameter,
                           const GeoCoord &targetCoord,
                           const Vehicle &vehicle,
                           size_t &nodesIgnoredCount,
                           double &currentMaxDistance,
                           const double &overallDistance,
                           const double &costLimit);
  public:
    explicit AbstractRoutingService(const RouterParameter& parameter);
    ~AbstractRoutingService() override;

    RoutingResult CalculateRoute(RoutingState& state,
                                 const RoutePosition& start,
                                 const RoutePosition& target,
                                 const RoutingParameter& parameter);

    bool TransformRouteDataToRouteDescription(const RouteData& data,
                                              RouteDescription& description);

    bool TransformRouteDataToPoints(const RouteData& data,
                                    std::list<Point>& points);

    bool TransformRouteDataToWay(const RouteData& data,
                                 Way& way);
  };

}

#endif /* OSMSCOUT_ABSTRACTROUTINGSERVICE_H */
