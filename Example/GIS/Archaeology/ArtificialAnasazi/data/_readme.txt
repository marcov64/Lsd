This folder contains the model by Janssen 2009 in version 2.0/1.1 as from
https://www.comses.net/codebases/2222/releases/1.1.0/

with some small modifications:
  a There was a bug (the zones names) which can be "switched off"
    Another a bug (negative harvest possible) was not fixed, for it does not
    change the data needed for validation.
  b Options were added to export the GIS data in the applied form,
    to facilitate cross-validation
  c In addition the graphical views where enhanced and an option was
    added to allow creating views for each data, again for cross-validation

    the data is:
      water sources,
      apdsi,
      yield (function of zones and apdsi) and
      historical population (in #households)

    the data is created as "*_janssen.txt", following the same order as the
    input data for the map: the "grid" is 80x120,
    the data is one line per year,
    starting at position 0,119 and then moving by column:
     (0,118 ... 0,0 ; 1,119 ... 79,0)

Switches:
  validation_mode on [added]
  export_views_mode on [added]
  ZONES_BUG off (contrary to Janssen.2009) [added]
  historicview on

Settings:
  harvestAdjustment: 0.56
  harvestVariance: 0.40
  DeathAge: 38
  FertilityEndsAge: 34
  Fertility: 0.155
