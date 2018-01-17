import os
import shutil
import numpy as np
import flopy
import pyemu

#nlay, nrow, ncol = 1, 250, 250
nlay, nrow, ncol = 116, 78, 59

def setup():


    nper = 1
    perlen = 1.0
    nstp = 1
    #botm = np.arange(-10, (nlay)*10, -10)
    botm = np.linspace(-10,-100,nlay)
    m = flopy.modflow.Modflow("pest", version="mfnwt", exe_name="mfnwt", external_path='.', verbose=True)
    steady = [False for _ in range(nper)]
    steady[0] = True
    flopy.modflow.ModflowDis(m, nlay=nlay, nrow=nrow, ncol=ncol, nper=nper,
                             perlen=perlen, nstp=nstp, top=0.0, botm=botm, steady=steady)

    flopy.modflow.ModflowBas(m, strt=0.0, ibound=1)

    wel_step = int(nrow / 20)

    wel_i = [i for i in range(wel_step, nrow, wel_step)]
    wel_j = [j for j in range(wel_step, ncol, wel_step)]
    wel_max = 0
    wel_min = -1000

    wel_data = {}
    for iper in range(nper):
        wd = []
        fluxes = np.random.uniform(wel_min, wel_max, len(wel_i))
        for i, j, f in zip(wel_i, wel_j, fluxes):
            wd.append([nlay - 1, i, j, f])
        wel_data[iper] = wd

    flopy.modflow.ModflowWel(m, stress_period_data=wel_data, ipakcb=50)

    ghb_stage1 = np.random.uniform(-9, 0, nper)
    ghb_stage2 = np.random.uniform(-9, 0, nper)
    ghb_stage_sw = np.random.uniform(-1, 0, nper)
    ghb_data = {}
    for iper, (g1, g2, gs) in enumerate(zip(ghb_stage1, ghb_stage2, ghb_stage_sw)):
        gd = []
        for i in range(0, int(nrow / 2)):
            gd.append([0, i, 0, g1, 1.0])
        for i in range(int(nrow / 2), nrow):
            gd.append([0, i, ncol - 1, g2, 10.0])
        for j in range(wel_step, ncol, wel_step):
            for i in range(nrow):
                gd.append([0, i, j, gs, 5.0])
        ghb_data[iper] = gd
    flopy.modflow.ModflowGhb(m, stress_period_data=ghb_data, ipakcb=50)

    hk = 10.0 ** (np.random.randn(nlay, nrow, ncol) * 0.01)
    vka = 10.0 ** (np.random.randn(nlay, nrow, ncol) * 0.01)
    #print(hk.min(), hk.max(), hk.mean(), hk.std())
    #hk = np.loadtxt("hk.dat")
    #vka = hk / 10.0
    flopy.modflow.ModflowUpw(m, hk=hk, vka=vka, ss=0.001, sy=0.1, ipakcb=50)

    flopy.modflow.ModflowOc(m, save_every=True,
                            save_types=["save head"])  # ,"save budget","print budget"])#stress_period_data=None)

    flopy.modflow.ModflowNwt(m, iprnwt=1, headtol=0.5)
    m.model_ws = "temp"
    m.write_input()
    m.run_model()
    grid_props = []
    hds_kperk = [[0,0]]
    #grid_props.append(["upw.hk",0])
    for k in range(m.nlay):
        grid_props.append(["upw.hk",k])
        grid_props.append(["upw.vka",k])
        grid_props.append(["upw.ss",k])

    m.upw.hk = hk.mean()
    m.upw.vka = vka.mean()
    ph = pyemu.helpers.PstFromFlopyModel(m,new_model_ws="template",grid_props=grid_props,hds_kperk=hds_kperk,
                                    model_exe_name="mfnwt",build_prior=False,remove_existing=True)

    ph.pst.observation_data.loc[:,"weight"] = 0.0
    obs = ph.pst.observation_data
    hds_obs = obs.loc[obs.obgnme=="hds",:]

    obs_idx = np.random.randint(0,hds_obs.shape[0],250)
    nz_obs_names = hds_obs.obsnme.iloc[obs_idx]

    ph.pst.observation_data.loc[nz_obs_names,"weight"] = 1.0
    #noise = np.random.randn(ph.pst.nnz_obs)
    #ph.pst.observation_data.loc[ph.pst.nnz_obs_names,"obsval"] += noise
    ph.pst.write(os.path.join("template","pest.pst"))

    num_reals = 100
    parcov = pyemu.Cov.from_parameter_data(ph.pst)
    pe = pyemu.ParameterEnsemble.from_gaussian_draw(ph.pst,parcov,num_reals=num_reals)
    #pe.to_csv(os.path.join("template","par.csv"))
    pe.to_binary(os.path.join("template", "par.jcb"))

    oe = pyemu.ObservationEnsemble.from_id_gaussian_draw(ph.pst,num_reals=num_reals)
    oe.to_binary(os.path.join("template","obs.jcb"))

    oe = pyemu.ObservationEnsemble.from_id_gaussian_draw(ph.pst, num_reals=num_reals)
    oe.to_binary(os.path.join("template", "restart_obs.jcb"))

    pyemu.helpers.run("pestpp pest.pst",cwd="template")


def prep():
    if os.path.exists("master"):
       shutil.rmtree("master")
    shutil.copytree("template","master")
    pst = pyemu.Pst(os.path.join("master","pest.pst"))
    pst.pestpp_options["ies_parameter_csv"] = "par.jcb"
    pst.pestpp_options["ies_observation_csv"] = "obs.jcb"
    pst.pestpp_options["ies_obs_restart_csv"] = "restart_obs.jcb"
    pst.pestpp_options["ies_use_approx"] = "true"
    pst.svd_data.eigthresh = 1.0e-5
    pst.svd_data.maxsing = 1.0e+6
    pst.control_data.noptmax = 1
    pst.write(os.path.join("master","pest.pst"))


# def run_fieldgen():
#     d = "reals"
#     # if os.path.exists(d):
#     #     shutil.rmtree(d)
#     # os.mkdir(d)
#
#     with open(os.path.join(d,"settings.fig"),'w') as f:
#         f.write("date=dd/mm/yyyy\ncolrow=no\n")
#
#     with open(os.path.join(d,"grid.spc"),'w') as f:
#         f.write("{0} {1}\n0.0 0.0 0.0\n".format(nrow,ncol))
#         [f.write("1.0 ") for _ in range(ncol)]
#         f.write('\n')
#         [f.write("1.0 ") for _ in range(ncol)]
#         f.write('\n')
#
#     np.savetxt(os.path.join(d,"zone.dat"),np.ones((nrow,ncol)),fmt="%3d")
#
#     v = pyemu.geostats.ExpVario(1.0,300.0,2.5,45.0)
#     gs = pyemu.geostats.GeoStruct(variograms=[v])
#     gs.to_struct_file(os.path.join(d,"struct.dat"))
#     num_reals = 10
#     with open(os.path.join(d,"fieldgen.in"),'w') as f:
#         f.write('grid.spc\n\n')
#         f.write("zone.dat\nf\n")
#         f.write("struct.dat\nstruct1\no\n")
#         f.write("10\n")
#         f.write("{0}\n".format(num_reals))
#         f.write("real_\nf\n")
#         f.write('20\n\n')
#
#     pyemu.helpers.run("fieldgen <fieldgen.in",cwd=d)
#
#     import matplotlib.pyplot as plt
#
#     rfiles = [f for f in os.listdir(d) if "real_" in f]
#     for r in rfiles:
#         arr =np.loadtxt()

def process_training_image():
    if not os.path.exists("_data"):
        os.mkdir("_data")
    ti_file = "ti_strebelle.sgems.txt"
    ti_file = "TI1.SGEMS"
    with open(ti_file,'r') as f:
        line = f.readline()
        nrow,ncol,nlay = [int(i) for i in line.strip().split()]
        f.readline()
        f.readline()
        arr = np.loadtxt(f).reshape(nlay,nrow,ncol)
    # import matplotlib.pyplot as plt
    # plt.imshow(arr)
    # plt.show()
    vd = {0:0.01,1:2.0,2:50.0,3:250.0}
    hk = np.zeros_like(arr) + vd[0]
    for i,k in vd.items():
        hk[arr==i] = k


    for k in range(nlay):
        np.savetxt(os.path.join("_data","hk_{0}.dat".format(k)),hk[1,:,:],fmt="%15.6E")
    import matplotlib.pyplot as plt
    plt.imshow(arr[0,:,:])
    plt.show()

    return hk


if __name__ == "__main__":
    process_training_image()
    #setup()
    #prep()
