#ifndef OSMSCOUT_STYLECONFIG_H
#define OSMSCOUT_STYLECONFIG_H

/*
  This source is part of the libosmscout library
  Copyright (C) 2009  Tim Teulings

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

#include <limits>
#include <vector>

#include <osmscout/private/MapImportExport.h>

#include <osmscout/Coord.h>
#include <osmscout/Types.h>
#include <osmscout/TypeSet.h>

#include <osmscout/Node.h>
#include <osmscout/Area.h>
#include <osmscout/Way.h>

#include <osmscout/TypeConfig.h>

#include <osmscout/util/Color.h>
#include <osmscout/util/Reference.h>
#include <osmscout/util/Transformation.h>

namespace osmscout {
  class OSMSCOUT_MAP_API StyleVariable : public Referencable
  {
  public:
    StyleVariable();
    virtual ~StyleVariable();
  };

  typedef Ref<StyleVariable> StyleVariableRef;

  class OSMSCOUT_MAP_API StyleVariableColor : public StyleVariable
  {
  private:
    Color color;

  public:
    StyleVariableColor(const Color& color);

    inline const Color& GetColor()
    {
      return color;
    }
  };

  class OSMSCOUT_API SizeCondition : public Referencable
  {
  private:
    double minMM;
    bool minMMSet;

    double minPx;
    bool minPxSet;

    double maxMM;
    bool maxMMSet;

    double maxPx;
    bool maxPxSet;
  public:
    SizeCondition();
    virtual ~SizeCondition();

    void SetMinMM(double minMM);
    void SetMinPx(double minPx);

    void SetMaxMM(double maxMM);
    void SetMaxPx(double maxPx);

    bool Evaluate(double meterInPixel, double meterInMM) const;
  };

  typedef Ref<SizeCondition> SizeConditionRef;

  /**
   * Holds the all accumulated filter conditions as defined in the style sheet
   * for a style.
   */
  class OSMSCOUT_MAP_API StyleFilter
  {
  public:

  private:
    TypeSet               types;
    size_t                minLevel;
    size_t                maxLevel;
    bool                  bridge;
    bool                  tunnel;
    bool                  oneway;
    SizeConditionRef      sizeCondition;

  public:
    StyleFilter();
    StyleFilter(const StyleFilter& other);

    StyleFilter& SetTypes(const TypeSet& types);

    StyleFilter& SetMinLevel(size_t level);
    StyleFilter& SetMaxLevel(size_t level);
    StyleFilter& SetBridge(bool bridge);
    StyleFilter& SetTunnel(bool tunnel);
    StyleFilter& SetOneway(bool oneway);

    StyleFilter& SetSizeCondition(SizeCondition* condition);

    inline bool HasTypes() const
    {
      return types.HasTypes();
    }

    inline bool HasType(TypeId typeId) const
    {
      return types.IsTypeSet(typeId);
    }

    inline size_t GetMinLevel() const
    {
      return minLevel;
    }

    inline size_t GetMaxLevel() const
    {
      return maxLevel;
    }

    inline bool GetBridge() const
    {
      return bridge;
    }

    inline bool GetTunnel() const
    {
      return tunnel;
    }

    inline bool GetOneway() const
    {
      return oneway;
    }

    inline bool HasMaxLevel() const
    {
      return maxLevel!=std::numeric_limits<size_t>::max();
    }

    inline const SizeConditionRef& GetSizeCondition() const
    {
      return sizeCondition;
    }
  };

  /**
   * Holds all filter criteria (minus type and zoomlevel criteria which are
   * directly handled by the lookup table) for a concrete style which have to
   * evaluated during runtime.
   */
  class OSMSCOUT_MAP_API StyleCriteria
  {
  public:

  private:
    bool             bridge;
    bool             tunnel;
    bool             oneway;
    SizeConditionRef sizeCondition;

  public:
    StyleCriteria();
    StyleCriteria(const StyleFilter& other);
    StyleCriteria(const StyleCriteria& other);

    bool operator==(const StyleCriteria& other) const;
    bool operator!=(const StyleCriteria& other) const;

    inline bool GetBridge() const
    {
      return bridge;
    }

    inline bool GetTunnel() const
    {
      return tunnel;
    }

    inline bool GetOneway() const
    {
      return oneway;
    }

   bool Matches(double meterInPixel,
                double meterInMM) const;
   bool Matches(const AreaAttributes& attributes,
                double meterInPixel,
                double meterInMM) const;
   bool Matches(const WayAttributes& attributes,
                double meterInPixel,
                double meterInMM) const;
  };

  /**
   * A Style together with a set of the attributes that are explicitely
   * set in the stye.
   */
  template<class S, class A>
  struct PartialStyle
  {
    std::set<A>    attributes;
    Ref<S>         style;

    PartialStyle()
    : style(new S())
    {
      // no code
    }
  };

  /**
   * A line in a style sheet. Connecting a set of filter criteria together with
   * a partial style definition.
   */
  template<class S, class A>
  struct ConditionalStyle
  {
    StyleFilter       filter;
    PartialStyle<S,A> style;

    ConditionalStyle(const StyleFilter& filter,
                     const PartialStyle<S,A>& style)
    : filter(filter),
      style(style)
    {
      // no code
    }
  };

  /**
   * Correlation of a StyleFilter and a PartialStyle. For an object
   * (node, way, area) all ConditionalStyle styles matching the criteria
   * are summed up to build the final style attribute set.
   */
  template<class S, class A>
  struct StyleSelector
  {
    StyleCriteria  criteria;
    std::set<A>    attributes;
    Ref<S>         style;

    StyleSelector(const StyleFilter& filter,
                  const PartialStyle<S,A>& style)
    : criteria(filter),
      attributes(style.attributes),
      style(style.style)
    {
      // no code
    }
  };

  /**
   * Style options for a line.
   */
  class OSMSCOUT_MAP_API LineStyle : public Referencable
  {
  public:
    enum CapStyle {
      capButt,
      capRound,
      capSquare
    };

    enum Attribute {
      attrLineColor,
      attrGapColor,
      attrDisplayWidth,
      attrWidth,
      attrDisplayOffset,
      attrOffset,
      attrJoinCap,
      attrEndCap,
      attrDashes,
      attrPriority
    };

  private:
    std::string         slot;
    Color               lineColor;
    Color               gapColor;
    double              displayWidth;
    double              width;
    double              displayOffset;
    double              offset;
    CapStyle            joinCap;
    CapStyle            endCap;
    std::vector<double> dash;
    int                 priority;

  public:
    LineStyle();
    LineStyle(const LineStyle& style);

    LineStyle& SetSlot(const std::string& slot);

    LineStyle& SetLineColor(const Color& color);
    LineStyle& SetGapColor(const Color& color);
    LineStyle& SetDisplayWidth(double value);
    LineStyle& SetWidth(double value);
    LineStyle& SetDisplayOffset(double value);
    LineStyle& SetOffset(double value);
    LineStyle& SetJoinCap(CapStyle joinCap);
    LineStyle& SetEndCap(CapStyle endCap);
    LineStyle& SetDashes(const std::vector<double> dashes);
    LineStyle& SetPriority(int priority);

    inline bool IsVisible() const
    {
      return (displayWidth>0.0 ||
              width>0.0) &&
              lineColor.IsVisible();
    }

    inline const std::string& GetSlot() const
    {
      return slot;
    }

    inline const Color& GetLineColor() const
    {
      return lineColor;
    }

    inline const Color& GetGapColor() const
    {
      return gapColor;
    }

    inline double GetDisplayWidth() const
    {
      return displayWidth;
    }

    inline double GetWidth() const
    {
      return width;
    }

    inline double GetDisplayOffset() const
    {
      return displayOffset;
    }

    inline double GetOffset() const
    {
      return offset;
    }

    inline CapStyle GetJoinCap() const
    {
      return joinCap;
    }

    inline CapStyle GetEndCap() const
    {
      return endCap;
    }

    inline bool HasDashes() const
    {
      return !dash.empty();
    }

    inline const std::vector<double>& GetDash() const
    {
      return dash;
    }

    inline int GetPriority() const
    {
      return priority;
    }

    void CopyAttributes(const LineStyle& other,
                        const std::set<Attribute>& attributes);
  };

  typedef Ref<LineStyle>                                   LineStyleRef;
  typedef PartialStyle<LineStyle,LineStyle::Attribute>     LinePartialStyle;
  typedef ConditionalStyle<LineStyle,LineStyle::Attribute> LineConditionalStyle;
  typedef StyleSelector<LineStyle,LineStyle::Attribute>    LineStyleSelector;
  typedef std::list<LineStyleSelector>                     LineStyleSelectorList; //! List of selectors
  typedef std::vector<std::vector<LineStyleSelectorList> > LineStyleLookupTable;  //!Index selectors by type and level

  /**
   * Style options for filling an area.
   */
  class OSMSCOUT_MAP_API FillStyle : public Referencable
  {
  public:
    enum Attribute {
      attrFillColor,
      attrPattern,
      attrPatternMinMag,
      attrBorderColor,
      attrBorderWidth,
      attrBorderDashes
    };

  private:
    Color               fillColor;
    std::string         pattern;
    mutable size_t      patternId;
    Magnification       patternMinMag;
    Color               borderColor;
    double              borderWidth;
    std::vector<double> borderDash;

  public:
    FillStyle();
    FillStyle(const FillStyle& style);

    FillStyle& SetFillColor(const Color& color);
    FillStyle& SetPattern(const std::string& pattern);
    void SetPatternId(size_t id) const;
    FillStyle& SetPatternMinMag(const Magnification& mag);
    FillStyle& SetBorderColor(const Color& color);
    FillStyle& SetBorderWidth(double value);
    FillStyle& SetBorderDashes(const std::vector<double> dashes);

    inline bool IsVisible() const
    {
      return (fillColor.IsVisible() ||
              (borderWidth>0 && borderColor.IsVisible()) ||
              !pattern.empty());
    }

    inline const Color& GetFillColor() const
    {
      return fillColor;
    }

    inline bool HasPattern() const
    {
      return !pattern.empty();
    }

    inline std::string GetPatternName() const
    {
      return pattern;
    }

    inline size_t GetPatternId() const
    {
      return patternId;
    }

    inline const Magnification& GetPatternMinMag() const
    {
      return patternMinMag;
    }

    inline const Color& GetBorderColor() const
    {
      return borderColor;
    }

    inline double GetBorderWidth() const
    {
      return borderWidth;
    }

    inline bool HasBorderDashes() const
    {
      return !borderDash.empty();
    }

    inline const std::vector<double>& GetBorderDash() const
    {
      return borderDash;
    }

    void CopyAttributes(const FillStyle& other,
                        const std::set<Attribute>& attributes);
  };

  typedef Ref<FillStyle>                                   FillStyleRef;
  typedef PartialStyle<FillStyle,FillStyle::Attribute>     FillPartialStyle;
  typedef ConditionalStyle<FillStyle,FillStyle::Attribute> FillConditionalStyle;
  typedef StyleSelector<FillStyle,FillStyle::Attribute>    FillStyleSelector;
  typedef std::list<FillStyleSelector>                     FillStyleSelectorList; //! List of selectors
  typedef std::vector<std::vector<FillStyleSelectorList> > FillStyleLookupTable;  //!Index selectors by type and level

  /**
   * Abstract base class for all (point) labels. All point labels have priority
   * and a alpha value.
   */
  class OSMSCOUT_MAP_API LabelStyle : public Referencable
  {
  private:
    uint8_t priority;
    double  size;

  public:
    LabelStyle();
    LabelStyle(const LabelStyle& style);
    virtual ~LabelStyle();

    virtual bool IsVisible() const = 0;
    virtual double GetAlpha() const = 0;

    LabelStyle& SetPriority(uint8_t priority);
    LabelStyle& SetSize(double size);

    inline uint8_t GetPriority() const
    {
      return priority;
    }

    inline double GetSize() const
    {
      return size;
    }
  };

  typedef Ref<LabelStyle> LabelStyleRef;

  /**
   * A textual label.
   */
  class OSMSCOUT_MAP_API TextStyle : public LabelStyle
  {
  public:
    enum Style {
      normal,
      emphasize
    };

    enum Label {
      none,
      name,
      ref
    };

    enum Attribute {
      attrPriority,
      attrSize,
      attrLabel,
      attrTextColor,
      attrStyle,
      attrScaleAndFadeMag
    };

  private:
    Style         style;
    Magnification scaleAndFadeMag;
    Label         label;
    Color         textColor;

  public:
    TextStyle();
    TextStyle(const TextStyle& style);

    TextStyle& SetPriority(uint8_t priority);
    TextStyle& SetSize(double size);
    TextStyle& SetLabel(Label label);
    TextStyle& SetTextColor(const Color& color);
    TextStyle& SetStyle(Style style);
    TextStyle& SetScaleAndFadeMag(const Magnification& mag);

    inline bool IsVisible() const
    {
      return label!=none &&
             GetTextColor().IsVisible();
    }

    inline double GetAlpha() const
    {
      return textColor.GetA();
    }

    inline Label GetLabel() const
    {
      return label;
    }

    inline const Color& GetTextColor() const
    {
      return textColor;
    }

    inline const Style& GetStyle() const
    {
      return style;
    }

    inline Magnification GetScaleAndFadeMag() const
    {
      return scaleAndFadeMag;
    }

    void CopyAttributes(const TextStyle& other,
                        const std::set<Attribute>& attributes);
  };

  typedef Ref<TextStyle>                                   TextStyleRef;
  typedef PartialStyle<TextStyle,TextStyle::Attribute>     TextPartialStyle;
  typedef ConditionalStyle<TextStyle,TextStyle::Attribute> TextConditionalStyle;
  typedef StyleSelector<TextStyle,TextStyle::Attribute>    TextStyleSelector;
  typedef std::list<TextStyleSelector>                     TextStyleSelectorList; //! List of selectors
  typedef std::vector<std::vector<TextStyleSelectorList> > TextStyleLookupTable;  //!Index selectors by type and level

  /**
   * A shield or plate label (text placed on a plate).
   */
  class OSMSCOUT_MAP_API ShieldStyle : public LabelStyle
  {
  public:
    enum Label {
      none,
      name,
      ref
    };

    enum Attribute {
      attrPriority,
      attrSize,
      attrLabel,
      attrTextColor,
      attrBgColor,
      attrBorderColor
    };

  private:
    Label   label;
    Color   textColor;
    Color   bgColor;
    Color   borderColor;

  public:
    ShieldStyle();
    ShieldStyle(const ShieldStyle& style);

    ShieldStyle& SetLabel(Label label);
    ShieldStyle& SetPriority(uint8_t priority);
    ShieldStyle& SetSize(double size);
    ShieldStyle& SetTextColor(const Color& color);
    ShieldStyle& SetBgColor(const Color& color);
    ShieldStyle& SetBorderColor(const Color& color);

    inline bool IsVisible() const
    {
      return label!=none &&
             GetTextColor().IsVisible();
    }

    inline double GetAlpha() const
    {
      return textColor.GetA();
    }

    inline Label GetLabel() const
    {
      return label;
    }

    inline const Color& GetTextColor() const
    {
      return textColor;
    }

    inline const Color& GetBgColor() const
    {
      return bgColor;
    }

    inline const Color& GetBorderColor() const
    {
      return borderColor;
    }

    void CopyAttributes(const ShieldStyle& other,
                        const std::set<Attribute>& attributes);
  };

  typedef Ref<ShieldStyle>                                     ShieldStyleRef;
  typedef PartialStyle<ShieldStyle,ShieldStyle::Attribute>     ShieldPartialStyle;
  typedef ConditionalStyle<ShieldStyle,ShieldStyle::Attribute> ShieldConditionalStyle;
  typedef StyleSelector<ShieldStyle,ShieldStyle::Attribute>    ShieldStyleSelector;
  typedef std::list<ShieldStyleSelector>                       ShieldStyleSelectorList; //! List of selectors
  typedef std::vector<std::vector<ShieldStyleSelectorList> >   ShieldStyleLookupTable;  //!Index selectors by type and level

  /**
   * A stle definng repretive drawing of a shiled label along a path. It consists
   * mainly of the attributes of the shield itself (it internally holds a shield
   * label for this) and some more attributes defining the way of repetition.
   */
  class OSMSCOUT_MAP_API PathShieldStyle : public Referencable
  {
  public:
    enum Attribute {
      attrPriority,
      attrSize,
      attrLabel,
      attrTextColor,
      attrBgColor,
      attrBorderColor,
      attrShieldSpace
    };

  private:
    ShieldStyleRef shieldStyle;
    double         shieldSpace;

  public:
    PathShieldStyle();
    PathShieldStyle(const PathShieldStyle& style);

    PathShieldStyle& SetLabel(ShieldStyle::Label label);
    PathShieldStyle& SetPriority(uint8_t priority);
    PathShieldStyle& SetSize(double size);
    PathShieldStyle& SetTextColor(const Color& color);
    PathShieldStyle& SetBgColor(const Color& color);
    PathShieldStyle& SetBorderColor(const Color& color);
    PathShieldStyle& SetShieldSpace(double shieldSpace);

    inline bool IsVisible() const
    {
      return shieldStyle->IsVisible();
    }

    inline double GetAlpha() const
    {
      return shieldStyle->GetAlpha();
    }

    inline uint8_t GetPriority() const
    {
      return shieldStyle->GetPriority();
    }

    inline double GetSize() const
    {
      return shieldStyle->GetSize();
    }

    inline ShieldStyle::Label GetLabel() const
    {
      return shieldStyle->GetLabel();
    }

    inline const Color& GetTextColor() const
    {
      return shieldStyle->GetTextColor();
    }

    inline const Color& GetBgColor() const
    {
      return shieldStyle->GetBgColor();
    }

    inline const Color& GetBorderColor() const
    {
      return shieldStyle->GetBorderColor();
    }

    inline double GetShieldSpace() const
    {
      return shieldSpace;
    }

    inline const ShieldStyleRef& GetShieldStyle() const
    {
      return shieldStyle;
    }

    void CopyAttributes(const PathShieldStyle& other,
                        const std::set<Attribute>& attributes);
  };

  typedef Ref<PathShieldStyle>                                         PathShieldStyleRef;
  typedef PartialStyle<PathShieldStyle,PathShieldStyle::Attribute>     PathShieldPartialStyle;
  typedef ConditionalStyle<PathShieldStyle,PathShieldStyle::Attribute> PathShieldConditionalStyle;
  typedef StyleSelector<PathShieldStyle,PathShieldStyle::Attribute>    PathShieldStyleSelector;
  typedef std::list<PathShieldStyleSelector>                           PathShieldStyleSelectorList; //! List of selectors
  typedef std::vector<std::vector<PathShieldStyleSelectorList> >       PathShieldStyleLookupTable;  //!Index selectors by type and level

  /**
   * A style for drawing text onto a path, the text following the
   * contour of the path.
   */
  class OSMSCOUT_MAP_API PathTextStyle : public Referencable
  {
  public:
    enum Label {
      none,
      name,
      ref
    };

    enum Attribute {
      attrLabel,
      attrSize,
      attrTextColor
    };

  private:
    Label   label;
    double  size;
    Color   textColor;

  public:
    PathTextStyle();
    PathTextStyle(const PathTextStyle& style);

    PathTextStyle& SetLabel(Label label);
    PathTextStyle& SetSize(double size);
    PathTextStyle& SetTextColor(const Color& color);

    inline bool IsVisible() const
    {
      return label!=none &&
             textColor.IsVisible();
    }

    inline Label GetLabel() const
    {
      return label;
    }

    inline double GetSize() const
    {
      return size;
    }

    inline const Color& GetTextColor() const
    {
      return textColor;
    }

    void CopyAttributes(const PathTextStyle& other,
                        const std::set<Attribute>& attributes);
  };

  typedef Ref<PathTextStyle>                                       PathTextStyleRef;
  typedef PartialStyle<PathTextStyle,PathTextStyle::Attribute>     PathTextPartialStyle;
  typedef ConditionalStyle<PathTextStyle,PathTextStyle::Attribute> PathTextConditionalStyle;
  typedef StyleSelector<PathTextStyle,PathTextStyle::Attribute>    PathTextStyleSelector;
  typedef std::list<PathTextStyleSelector>                         PathTextStyleSelectorList; //! List of selectors
  typedef std::vector<std::vector<PathTextStyleSelectorList> >     PathTextStyleLookupTable;  //!Index selectors by type and level

  class OSMSCOUT_MAP_API DrawPrimitive : public Referencable
  {
  private:
    FillStyleRef fillStyle;

  public:
    DrawPrimitive(const FillStyleRef& fillStyle);
    virtual ~DrawPrimitive();

    inline const FillStyleRef& GetFillStyle() const
    {
      return fillStyle;
    }

    virtual void GetBoundingBox(double& minX,
                                double& minY,
                                double& maxX,
                                double& maxY) const = 0;
  };

  typedef Ref<DrawPrimitive> DrawPrimitiveRef;

  class OSMSCOUT_MAP_API PolygonPrimitive : public DrawPrimitive
  {
  private:
    std::list<Coord> coords;

  public:
    PolygonPrimitive(const FillStyleRef& fillStyle);

    void AddCoord(const Coord& coord);

    inline const std::list<Coord>& GetCoords() const
    {
      return coords;
    }

    void GetBoundingBox(double& minX,
                        double& minY,
                        double& maxX,
                        double& maxY) const;
  };

  typedef Ref<PolygonPrimitive> PolygonPrimitiveRef;

  class OSMSCOUT_MAP_API RectanglePrimitive : public DrawPrimitive
  {
  private:
    Coord  topLeft;
    double width;
    double height;

  public:
    RectanglePrimitive(const Coord& topLeft,
                       double width,
                       double height,
                       const FillStyleRef& fillStyle);

    inline const Coord& GetTopLeft() const
    {
      return topLeft;
    }

    inline const double& GetWidth() const
    {
      return width;
    }

    inline const double& GetHeight() const
    {
      return height;
    }

    void GetBoundingBox(double& minX,
                        double& minY,
                        double& maxX,
                        double& maxY) const;
  };

  typedef Ref<RectanglePrimitive> RectanglePrimitiveRef;

  class OSMSCOUT_MAP_API CirclePrimitive : public DrawPrimitive
  {
  private:
    Coord  center;
    double radius;

  public:
    CirclePrimitive(const Coord& center,
                    double radius,
                    const FillStyleRef& fillStyle);

    inline const Coord& GetCenter() const
    {
      return center;
    }

    inline const double& GetRadius() const
    {
      return radius;
    }

    void GetBoundingBox(double& minX,
                        double& minY,
                        double& maxX,
                        double& maxY) const;
  };

  typedef Ref<CirclePrimitive> CirclePrimitiveRef;

  /**
   * Definition of a symbol. A symbol consists of a list of DrawPrimitives
   * with with assigned rendeirng styes.
   */
  class OSMSCOUT_MAP_API Symbol : public Referencable
  {
  private:
    std::string                 name;
    std::list<DrawPrimitiveRef> primitives;
    double                      minX;
    double                      minY;
    double                      maxX;
    double                      maxY;

  public:
    Symbol(const std::string& name);

    void AddPrimitive(const DrawPrimitiveRef& primitive);

    inline std::string GetName() const
    {
      return name;
    }

    inline const std::list<DrawPrimitiveRef>& GetPrimitives() const
    {
      return primitives;
    }

    inline void GetBoundingBox(double& minX,
                               double& minY,
                               double& maxX,
                               double& maxY) const
    {
      minX=this->minX;
      minY=this->minY;

      maxX=this->maxX;
      maxY=this->maxY;
    }

    inline double GetWidth() const
    {
      return maxX-minX;
    }

    inline double GetHeight() const
    {
      return maxY-minY;
    }
  };

  typedef Ref<Symbol> SymbolRef;

  /**
   * The icon style allow the rendering of external images or internal symbols.
   */
  class OSMSCOUT_MAP_API IconStyle : public Referencable
  {
  public:
    enum Attribute {
      attrSymbol,
      attrIconName
    };

  private:
    SymbolRef   symbol;
    std::string iconName; //! name of the icon as given in style
    size_t      iconId;   //! Id for external resource binding

  public:
    IconStyle();
    IconStyle(const IconStyle& style);

    IconStyle& SetSymbol(const SymbolRef& symbol);
    IconStyle& SetIconName(const std::string& iconName);
    IconStyle& SetIconId(size_t id);

    inline bool IsVisible() const
    {
      return !iconName.empty() ||
              symbol.Valid();
    }

    inline const SymbolRef& GetSymbol() const
    {
      return symbol;
    }

    inline std::string GetIconName() const
    {
      return iconName;
    }

    inline size_t GetIconId() const
    {
      return iconId;
    }

    void CopyAttributes(const IconStyle& other,
                        const std::set<Attribute>& attributes);
  };

  typedef Ref<IconStyle>                                   IconStyleRef;
  typedef PartialStyle<IconStyle,IconStyle::Attribute>     IconPartialStyle;
  typedef ConditionalStyle<IconStyle,IconStyle::Attribute> IconConditionalStyle;
  typedef StyleSelector<IconStyle,IconStyle::Attribute>    IconStyleSelector;
  typedef std::list<IconStyleSelector>                     IconStyleSelectorList; //! List of selectors
  typedef std::vector<std::vector<IconStyleSelectorList> > IconStyleLookupTable;  //!Index selectors by type and level

  /**
   * Style for repretive drawing of symbols on top of a path.
   */
  class OSMSCOUT_MAP_API PathSymbolStyle : public Referencable
  {
  public:

    enum Attribute {
      attrSymbol,
      attrSymbolSpace
    };

  private:
    SymbolRef symbol;
    double    symbolSpace;

  public:
    PathSymbolStyle();
    PathSymbolStyle(const PathSymbolStyle& style);

    PathSymbolStyle& SetSymbol(const SymbolRef& symbol);
    PathSymbolStyle& SetSymbolSpace(double space);

    inline bool IsVisible() const
    {
      return symbol.Valid();
    }

    inline const SymbolRef& GetSymbol() const
    {
      return symbol;
    }

    inline double GetSymbolSpace() const
    {
      return symbolSpace;
    }

    void CopyAttributes(const PathSymbolStyle& other,
                        const std::set<Attribute>& attributes);
  };

  typedef Ref<PathSymbolStyle>                                         PathSymbolStyleRef;
  typedef PartialStyle<PathSymbolStyle,PathSymbolStyle::Attribute>     PathSymbolPartialStyle;
  typedef ConditionalStyle<PathSymbolStyle,PathSymbolStyle::Attribute> PathSymbolConditionalStyle;
  typedef StyleSelector<PathSymbolStyle,PathSymbolStyle::Attribute>    PathSymbolStyleSelector;
  typedef std::list<PathSymbolStyleSelector>                           PathSymbolStyleSelectorList; //! List of selectors
  typedef std::vector<std::vector<PathSymbolStyleSelectorList> >       PathSymbolStyleLookupTable;  //!Index selectors by type and level

  /**
   * A complete style definition
   */
  class OSMSCOUT_MAP_API StyleConfig
  {
  private:
    TypeConfig                                 *typeConfig;

    // Symbol
    OSMSCOUT_HASHMAP<std::string,SymbolRef>    symbols;
    SymbolRef                                  emptySymbol;

    // Node

    std::list<TextConditionalStyle>            nodeTextStyleConditionals;
    std::list<IconConditionalStyle>            nodeIconStyleConditionals;

    TextStyleLookupTable                       nodeTextStyleSelectors;
    IconStyleLookupTable                       nodeIconStyleSelectors;

    std::vector<TypeSet>                       nodeTypeSets;

    // Way

    std::vector<size_t>                        wayPrio;

    std::list<LineConditionalStyle>            wayLineStyleConditionals;
    std::list<PathTextConditionalStyle>        wayPathTextStyleConditionals;
    std::list<PathSymbolConditionalStyle>      wayPathSymbolStyleConditionals;
    std::list<PathShieldConditionalStyle>      wayPathShieldStyleConditionals;

    std::vector<LineStyleLookupTable>          wayLineStyleSelectors;
    PathTextStyleLookupTable                   wayPathTextStyleSelectors;
    PathSymbolStyleLookupTable                 wayPathSymbolStyleSelectors;
    PathShieldStyleLookupTable                 wayPathShieldStyleSelectors;

    std::vector<std::vector<TypeSet> >         wayTypeSets;

    // Area

    std::list<FillConditionalStyle>            areaFillStyleConditionals;
    std::list<TextConditionalStyle>            areaTextStyleConditionals;
    std::list<IconConditionalStyle>            areaIconStyleConditionals;

    FillStyleLookupTable                       areaFillStyleSelectors;
    TextStyleLookupTable                       areaTextStyleSelectors;
    IconStyleLookupTable                       areaIconStyleSelectors;

    std::vector<TypeSet>                       areaTypeSets;

    OSMSCOUT_HASHMAP<std::string,StyleVariableRef> variables;


  private:
    void GetAllNodeTypes(std::list<TypeId>& types);
    void GetAllWayTypes(std::list<TypeId>& types);
    void GetAllAreaTypes(std::list<TypeId>& types);

    void PostprocessNodes();
    void PostprocessWays();
    void PostprocessAreas();
    void PostprocessIconId();
    void PostprocessPatternId();

  public:
    StyleConfig(TypeConfig* typeConfig);
    virtual ~StyleConfig();

    StyleVariableRef GetVariableByName(const std::string& name) const;
    void AddVariable(const std::string& name,
                     const StyleVariableRef& variable);

    bool RegisterSymbol(const SymbolRef& symbol);
    const SymbolRef& GetSymbol(const std::string& name) const;

    void Postprocess();

    TypeConfig* GetTypeConfig() const;

    StyleConfig& SetWayPrio(TypeId type, size_t prio);

    void AddNodeTextStyle(const StyleFilter& filter,
                          TextPartialStyle& stype);
    void AddNodeIconStyle(const StyleFilter& filter,
                          IconPartialStyle& style);

    void AddWayLineStyle(const StyleFilter& filter,
                         LinePartialStyle& style);
    void AddWayPathTextStyle(const StyleFilter& filter,
                             PathTextPartialStyle& style);
    void AddWayPathSymbolStyle(const StyleFilter& filter,
                               PathSymbolPartialStyle& style);
    void AddWayPathShieldStyle(const StyleFilter& filter,
                               PathShieldPartialStyle& style);

    void AddAreaFillStyle(const StyleFilter& filter,
                          FillPartialStyle& style);
    void AddAreaTextStyle(const StyleFilter& filter,
                          TextPartialStyle& style);
    void AddAreaIconStyle(const StyleFilter& filter,
                          IconPartialStyle& style);

    void GetNodeTypesWithMaxMag(const Magnification& maxMag,
                                TypeSet& types) const;
    void GetWayTypesByPrioWithMaxMag(const Magnification& mag,
                                     std::vector<TypeSet>& types) const;
    void GetAreaTypesWithMaxMag(const Magnification& maxMag,
                                TypeSet& types) const;


    inline size_t GetWayPrio(TypeId type) const
    {
      if (type<wayPrio.size()) {
        return wayPrio[type];
      }
      else {
        return std::numeric_limits<size_t>::max();
      }
    }

    void GetNodeTextStyle(const Node& node,
                          const Projection& projection,
                          double dpi,
                          TextStyleRef& textStyle) const;

    void GetNodeIconStyle(const Node& node,
                          const Projection& projection,
                          double dpi,
                          IconStyleRef& iconStyle) const;

    void GetWayLineStyles(const WayAttributes& way,
                          const Projection& projection,
                          double dpi,
                          std::vector<LineStyleRef>& lineStyles) const;
    void GetWayPathTextStyle(const WayAttributes& way,
                             const Projection& projection,
                             double dpi,
                             PathTextStyleRef& pathTextStyle) const;
    void GetWayPathSymbolStyle(const WayAttributes& way,
                               const Projection& projection,
                               double dpi,
                               PathSymbolStyleRef& pathSymbolStyle) const;
    void GetWayPathShieldStyle(const WayAttributes& way,
                               const Projection& projection,
                               double dpi,
                               PathShieldStyleRef& pathShieldStyle) const;

    void GetAreaFillStyle(const TypeId& type,
                          const AreaAttributes& area,
                          const Projection& projection,
                          double dpi,
                          FillStyleRef& fillStyle) const;
    void GetAreaTextStyle(const TypeId& type,
                          const AreaAttributes& area,
                          const Projection& projection,
                          double dpi,
                          TextStyleRef& textStyle) const;
    void GetAreaIconStyle(const TypeId& type,
                          const AreaAttributes& area,
                          const Projection& projection,
                          double dpi,
                          IconStyleRef& iconStyle) const;

    void GetLandFillStyle(const Projection& projection,
                          double dpi,
                          FillStyleRef& fillStyle) const;
    void GetSeaFillStyle(const Projection& projection,
                          double dpi,
                         FillStyleRef& fillStyle) const;
    void GetCoastFillStyle(const Projection& projection,
                          double dpi,
                           FillStyleRef& fillStyle) const;
    void GetUnknownFillStyle(const Projection& projection,
                             double dpi,
                             FillStyleRef& fillStyle) const;
    void GetCoastlineLineStyle(const Projection& projection,
                               double dpi,
                               LineStyleRef& lineStyle) const;
  };
}

#endif
