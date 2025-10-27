#
# A little script to replace
# all '{}' and '{ }' with '{ 0 }' to please MSVC.
#
# Also replaces '__MINGW32__' with '_WIN32'.
#
import sys, os, pathlib

def fix_file (file):
  in_file  = open (file, "rt")
  out_file = open (file + "-fixed", "w+t")
  count = 0

  for l in in_file.readlines():

    # Do not change lines like these:
    #   for (j = 0; j <= m; j++) { }
    #   do {} while(0)
    #
    if l.lstrip().startswith("for ("):
       out_file.write (l)
       continue

    if l.rstrip().endswith("do {} while(0)"):
       out_file.write (l)
       continue

    new_l = l.replace ("{}", "{ 0 }")
    if new_l != l:
       count += 1
       l = new_l

    new_l = l.replace ("{ }", "{ 0 }")
    if new_l != l:
       count += 1
       l = new_l

    new_l = l.replace ("__MINGW32__", "_WIN32")
    if new_l != l:
       count += 1
       l = new_l

    out_file.write (l)

  out_file.close()
  in_file.close()

  if count >= 1:
     print (f"Did {count} replacement on {file}.")
     os.unlink (file)
     os.rename (file + "-fixed", file)
     pathlib.Path (file).touch()
  else:
     pathlib.Path (file + "-fixed").unlink()
  return count

files = [ "src/conf.c",
          "src/rot_conf.c",
          "rigs/adat/adt_200a.c",
          "rigs/alinco/dx77.c",
          "rigs/alinco/dxsr8.c",
          "rigs/aor/ar2700.c",
          "rigs/aor/ar3000.c",
          "rigs/aor/ar3030.c",
          "rigs/aor/ar5000.c",
          "rigs/aor/ar7030.c",
          "rigs/aor/ar8000.c",
          "rigs/aor/ar8200.c",
          "rigs/aor/ar8600.c",
          "rigs/aor/sr2200.c",
          "rigs/anytone/d578.c",
          "rigs/dorji/dra818.c",
          "rigs/drake/r8a.c",
          "rigs/drake/r8b.c",
          "rigs/dummy/dummy.c",
          "rigs/dummy/flrig.c",
          "rigs/dummy/netrigctl.c",
          "rigs/dummy/netrigctl.c",
          "rigs/dummy/rot_dummy.c",
          "rigs/dummy/quisk.c",
          "rigs/dummy/tci1x.c",
          "rigs/elad/elad.c",
          "rigs/icmarine/icm700pro.c",
          "rigs/icmarine/icm710.c",
          "rigs/icmarine/icm802.c",
          "rigs/icmarine/icm803.c",
          "rigs/icom/delta2.c",
          "rigs/icom/delta2.c",
          "rigs/icom/ic1275.c",
          "rigs/icom/ic271.c",
          "rigs/icom/ic2730.c",
          "rigs/icom/ic275.c",
          "rigs/icom/ic471.c",
          "rigs/icom/ic471.c",
          "rigs/icom/ic475.c",
          "rigs/icom/ic7000.c",
          "rigs/icom/ic703.c",
          "rigs/icom/ic706.c",
          "rigs/icom/ic707.c",
          "rigs/icom/ic7100.c",
          "rigs/icom/ic718.c",
          "rigs/icom/ic7200.c",
          "rigs/icom/ic725.c",
          "rigs/icom/ic726.c",
          "rigs/icom/ic728.c",
          "rigs/icom/ic7300.c",
          "rigs/icom/ic735.c",
          "rigs/icom/ic736.c",
          "rigs/icom/ic736.c",
          "rigs/icom/ic737.c",
          "rigs/icom/ic738.c",
          "rigs/icom/ic738.c",
          "rigs/icom/ic7410.c",
          "rigs/icom/ic746.c",
          "rigs/icom/ic751.c",
          "rigs/icom/ic756.c",
          "rigs/icom/ic756.c",
          "rigs/icom/ic7600.c",
          "rigs/icom/ic761.c",
          "rigs/icom/ic7610.c",
          "rigs/icom/ic765.c",
          "rigs/icom/ic7700.c",
          "rigs/icom/ic775.c",
          "rigs/icom/ic78.c",
          "rigs/icom/ic7800.c",
          "rigs/icom/ic781.c",
          "rigs/icom/ic785x.c",
          "rigs/icom/ic820h.c",
          "rigs/icom/ic820h.c",
          "rigs/icom/ic821h.c",
          "rigs/icom/ic821h.c",
          "rigs/icom/ic910.c",
          "rigs/icom/ic9100.c",
          "rigs/icom/ic92d.c",
          "rigs/icom/ic970.c",
          "rigs/icom/ic970.c",
          "rigs/icom/icf8101.c",
          "rigs/icom/icom.c",
          "rigs/icom/icr10.c",
          "rigs/icom/icr20.c",
          "rigs/icom/icr30.c",
          "rigs/icom/icr6.c",
          "rigs/icom/icr7000.c",
          "rigs/icom/icr71.c",
          "rigs/icom/icr72.c",
          "rigs/icom/icr8500.c",
          "rigs/icom/icr9000.c",
          "rigs/icom/icr9500.c",
          "rigs/icom/icrx7.c",
          "rigs/icom/id1.c",
          "rigs/icom/id31.c",
          "rigs/icom/id4100.c",
          "rigs/icom/id51.c",
          "rigs/icom/id5100.c",
          "rigs/icom/omni.c",
          "rigs/icom/omni.c",
          "rigs/icom/optoscan.c",
          "rigs/icom/os456.c",
          "rigs/icom/os535.c",
          "rigs/icom/perseus.c",
          "rigs/icom/xiegu.c",
          "rigs/jrc/jst145.c",
          "rigs/jrc/nrd525.c",
          "rigs/jrc/nrd535.c",
          "rigs/jrc/nrd545.c",
          "rigs/kachina/505dsp.c",
          "rigs/kenwood/elecraft.c",
          "rigs/kenwood/flex6xxx.c",
          "rigs/kenwood/k2.c",
          "rigs/kenwood/k3.c",
          "rigs/kenwood/k3.c",
          "rigs/kenwood/kenwood.c",
          "rigs/kenwood/pihpsdr.c",
          "rigs/kenwood/r5000.c",
          "rigs/kenwood/thd7.c",
          "rigs/kenwood/thd72.c",
          "rigs/kenwood/thd74.c",
          "rigs/kenwood/thf6a.c",
          "rigs/kenwood/thf7.c",
          "rigs/kenwood/thg71.c",
          "rigs/kenwood/tmd700.c",
          "rigs/kenwood/tmd710.c",
          "rigs/kenwood/tmv7.c",
          "rigs/kenwood/transfox.c",
          "rigs/kenwood/trc80.c",
          "rigs/kenwood/ts140.c",
          "rigs/kenwood/ts2000.c",
          "rigs/kenwood/ts440.c",
          "rigs/kenwood/ts450s.c",
          "rigs/kenwood/ts450s.c",
          "rigs/kenwood/ts480.c",
          "rigs/kenwood/ts50s.c",
          "rigs/kenwood/ts570.c",
          "rigs/kenwood/ts680.c",
          "rigs/kenwood/ts690.c",
          "rigs/kenwood/ts711.c",
          "rigs/kenwood/ts790.c",
          "rigs/kenwood/ts811.c",
          "rigs/kenwood/ts850.c",
          "rigs/kenwood/ts850.c",
          "rigs/kenwood/ts870s.c",
          "rigs/kenwood/ts930.c",
          "rigs/kenwood/ts940.c",
          "rigs/kenwood/ts950.c",
          "rigs/kenwood/ts990s.c",
          "rigs/kenwood/tx500.c",
          "rigs/kenwood/xg3.c",
          "rigs/kit/dds60.c",
          "rigs/kit/drt1.c",
          "rigs/kit/dwt.c",
          "rigs/kit/elektor304.c",
          "rigs/kit/elektor507.c",
          "rigs/kit/fifisdr.c",
          "rigs/kit/funcube.c",
          "rigs/kit/si570avrusb.c",
          "rigs/kit/usrp.c",
          "rigs/tentec/orion.h",
          "rigs/lowe/hf235.c",
          "rigs/pcr/pcr100.c",
          "rigs/pcr/pcr1000.c",
          "rigs/pcr/pcr1500.c",
          "rigs/pcr/pcr2500.c",
          "rigs/rft/ekd500.c",
          "rigs/rs/eb200.c",
          "rigs/rs/ek89x.c",
          "rigs/rs/esmc.c",
          "rigs/rs/xk2100.c",
          "rigs/tapr/dsp10.c",
          "rigs/tentec/argonaut.c",
          "rigs/tentec/jupiter.c",
          "rigs/tentec/omnivii.c",
          "rigs/tentec/paragon.c",
          "rigs/tentec/pegasus.c",
          "rigs/tentec/rx320.c",
          "rigs/tentec/rx331.c",
          "rigs/tentec/rx340.c",
          "rigs/tentec/rx350.c",
          "rigs/uniden/bc245.c",
          "rigs/uniden/bc250.c",
          "rigs/uniden/bc780.c",
          "rigs/uniden/bc895.c",
          "rigs/uniden/bc898.c",
          "rigs/uniden/bcd396t.c",
          "rigs/uniden/bcd996t.c",
          "rigs/uniden/pro2052.c",
          "rigs/yaesu/ft100.c",
          "rigs/yaesu/ft600.c",
          "rigs/yaesu/ft817.c",
          "rigs/yaesu/ft847.c",
          "rigs/yaesu/ft857.c",
          "rigs/yaesu/ft897.c",
          "rigs/yaesu/vr5000.c",
          "rigs/yaesu/vx1700.c",
          "rotators/rotorez/rotorez.c"
        ]

for f in files:
  fixed_files = 0
  try:
    fixed_files += fix_file (f)
  except UnicodeDecodeError:
    print ("Failed for %s" %f)

print (f"Did {fixed_files} replacements in {len(files)} files.")
