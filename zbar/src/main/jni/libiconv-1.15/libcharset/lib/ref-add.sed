/^# Packages using this file: / {
  s/# Packages using this file://
  ta
  :a
  s/ @PACKAGE@ / @PACKAGE@ /
  tb
  s/ $/ @PACKAGE@ /
  :b
  s/^/# Packages using this file:/
}
