/*  
	� Copyright 2012, David Welter
	
	This file is part of PEST++.
   
	PEST++ is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	PEST++ is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with PEST++.  If not, see<http://www.gnu.org/licenses/>.
*/

#include <cstdlib>
#include <vector>
#include <set>
#include <fstream>
#include <algorithm>
#include "Jacobian_1to1.h"
#include "Transformable.h"
#include "ParamTransformSeq.h"
#include "pest_error.h"
#include "pest_data_structs.h"
#include "ModelRunPP.h"
#include "RunManagerAbstract.h"
#include "ObjectiveFunc.h"
#include "utilities.h"
#include "FileManager.h"
#include "PriorInformation.h"

using namespace std;
using namespace pest_utils;

Jacobian_1to1::Jacobian_1to1(FileManager &_file_manager) : Jacobian(_file_manager) 
{
}

Jacobian_1to1::~Jacobian_1to1() {
}

bool Jacobian_1to1::build_runs(ModelRun &init_model_run, vector<string> numeric_par_names, ParamTransformSeq &par_transform,
		const ParameterGroupInfo &group_info, const ParameterInfo &ctl_par_info, 
		RunManagerAbstract &run_manager, set<string> &out_of_bound_par, bool phiredswh_flag, bool calc_init_obs)
{
	Parameters model_parameters(par_transform.ctl2model_cp(init_model_run.get_ctl_pars()));
	Observations observations(init_model_run.get_obs_template());
	base_numeric_parameters = par_transform.ctl2numeric_cp( init_model_run.get_ctl_pars());
	run_manager.reinitialize(file_manager.build_filename("rnj"));

	failed_parameter_names.clear();
	// add base run
	int run_id = run_manager.add_run(model_parameters, "", 0);
	//if base run is has already been complete, update it and mark it as complete
	// compute runs for to jacobain calculation as it is influenced by derivative type( forward or central)
	if (!calc_init_obs) {
		run_manager.update_run(run_id, model_parameters, observations);
	}

	Parameters new_derivative_pars;
	bool success;
	Parameters base_derivative_parameters = par_transform.numeric2derivative_cp(base_numeric_parameters);
	//Loop through derivative parameters and build the parameter sets necessary for computing the jacobian
	for (auto &i_name : numeric_par_names)
	{
		assert(base_derivative_parameters.find(i_name) != base_derivative_parameters.end());
		vector<double> tmp_del_numeric_par_vec;
		double derivative_par_value = base_derivative_parameters.get_rec(i_name);
		success = get_derivative_parameters(i_name, derivative_par_value, par_transform, group_info, ctl_par_info,
			tmp_del_numeric_par_vec, phiredswh_flag);
		if (success && !tmp_del_numeric_par_vec.empty())
		{
			// update changed model parameters in model_parameters
			for (const auto &par : tmp_del_numeric_par_vec)
			{
				Parameters org_pars;
				Parameters new_pars;
				org_pars.insert(make_pair(i_name, model_parameters.get_rec(i_name)));
				new_pars.insert(make_pair(i_name, par));
				par_transform.derivative2model_ip(new_pars);
				for (auto &ipar : new_pars)
				{
					model_parameters[ipar.first] = ipar.second;
				}
				run_manager.add_run(model_parameters, i_name, par);
				for (const auto &ipar : org_pars)
				{
					model_parameters[ipar.first] = ipar.second;
				}
			}
		}
		else 
		{
			failed_parameter_names.insert(i_name);
		}
	}
	ofstream &fout_restart = file_manager.get_ofstream("rst");
	fout_restart << "jacobian_model_runs_built " << run_manager.get_cur_groupid() << endl;
	return true;
}


void Jacobian_1to1::make_runs(RunManagerAbstract &run_manager)
{
	// make model runs
	run_manager.run();
}

bool Jacobian_1to1::process_runs(vector<string> numeric_par_names, vector<string> obs_names, ParamTransformSeq &par_transform,
		const ParameterGroupInfo &group_info, const ParameterInfo &ctl_par_info, 
		RunManagerAbstract &run_manager,  const PriorInformation &prior_info, set<string> &out_of_bound_par, bool phiredswh_flag, bool calc_init_obs)
{
	base_sim_obs_names = obs_names;
	vector<string> prior_info_name = prior_info.get_keys();
	base_sim_obs_names.insert(base_sim_obs_names.end(), prior_info_name.begin(), prior_info_name.end());
	std::vector<Eigen::Triplet<double> > triplet_list;

	unordered_map<string, int> par2col_map = get_par2col_map();
	unordered_map<string, int>::iterator found;
	unordered_map<string, int>::iterator not_found = par2col_map.end();

	JacobianRun base_run;
	int i_run = 0;
	// get base run parameters and observation for initial model run from run manager storage
	bool success = run_manager.get_run(i_run, base_run.ctl_pars, base_run.obs);
	if (!success)
	{
		throw(PestError("Error: Base parameter run failed.  Can not compute the Jacobian"));
	}
	par_transform.model2ctl_ip(base_run.ctl_pars);
	base_numeric_parameters = par_transform.ctl2numeric_cp(base_run.ctl_pars);
	++i_run;

	// process the parameter pertubation runs
	int nruns = run_manager.get_nruns();
	base_numeric_par_names.clear();
	int icol = 0;
	vector<string>par_name_vec;
	string cur_par_name;
	string par_name_next;
	int run_status_next;
	double par_value_next;
	double cur_numeric_par_value;
	
	list<JacobianRun> run_list;
	for(; i_run<nruns; ++i_run)
	{
		run_list.push_back(JacobianRun());
		bool success = run_manager.get_run(i_run, run_list.back().ctl_pars, run_list.back().obs, cur_par_name, cur_numeric_par_value);
		run_list.back().numeric_derivative_par = cur_numeric_par_value;

		if (success)
		{
			par_transform.model2ctl_ip(run_list.back().ctl_pars);
			// get the updated parameter value which reflects roundoff errors
			par_name_vec.clear();
			par_name_vec.push_back(cur_par_name);
			Parameters numeric_pars(run_list.back().ctl_pars, par_name_vec);
			par_transform.ctl2numeric_ip(numeric_pars);
			run_list.back().numeric_derivative_par = numeric_pars.get_rec(cur_par_name);
		}
		else
		{
			run_list.pop_back();
		}

		// read information associated with the next model run;
		if (i_run+1<nruns)
		{
			run_manager.get_info(i_run+1, run_status_next, par_name_next, par_value_next);
		}

		if( i_run+1>=nruns || (cur_par_name !=par_name_next) )
		{
			if (!run_list.empty())
			{
				base_numeric_par_names.push_back(cur_par_name);
				base_run.numeric_derivative_par = base_numeric_parameters.get_rec(cur_par_name);
				run_list.push_front(base_run);
				std::vector<Eigen::Triplet<double> > tmp_triplet_vec = calc_derivative(cur_par_name, icol, run_list, group_info, prior_info);
				triplet_list.insert( triplet_list.end(), tmp_triplet_vec.begin(), tmp_triplet_vec.end() );
				icol++;
			}
			else
			{
				failed_parameter_names.insert(cur_par_name);
			}
			run_list.clear();
		}
	}
	matrix.resize(base_sim_obs_names.size(), base_numeric_par_names.size());
	matrix.setZero();
	matrix.setFromTriplets(triplet_list.begin(), triplet_list.end());
	// clean up
	ofstream &fout_restart = file_manager.get_ofstream("rst");
	fout_restart << "jacobian_saved" << endl;
	run_manager.free_memory();
	return true;
}

bool Jacobian_1to1::get_derivative_parameters(const string &par_name, double par_value, const ParamTransformSeq &par_trans, const ParameterGroupInfo &group_info, const ParameterInfo &ctl_par_info, 
		vector<double> &delta_numeric_par_vec, bool phiredswh_flag)
{
	bool success = false;
	const ParameterGroupRec *g_rec = group_info.get_group_rec_ptr(par_name);

	if (g_rec->forcen == "ALWAYS_3" || phiredswh_flag == true) {
		// Central Difference
		vector<double> new_par_vec;
		vector<Parameters> dir_numeric_pars_vec;
		success = central_diff(par_name, par_value, group_info, ctl_par_info, par_trans, new_par_vec, dir_numeric_pars_vec);
		if (success)
		{
			for (auto & ipar : new_par_vec)
			{
				delta_numeric_par_vec.push_back(ipar);
			}
		}
	}
	if (!success) {
		// Forward Difference
		success = forward_diff(par_name, par_value, group_info, ctl_par_info, par_trans, par_value);
		if(success)
		{
			delta_numeric_par_vec.push_back(par_value);
		}
	}
	return success;
}

bool Jacobian_1to1::forward_diff(const string &par_name, double base_derivative_val, 
		const ParameterGroupInfo &group_info, const ParameterInfo &ctl_par_info, const ParamTransformSeq &par_trans, double &new_par_val)
{
	const ParameterRec *par_info_ptr = ctl_par_info.get_parameter_rec_ptr(par_name);
	Parameters new_par;
	bool out_of_bound_forward;
	bool out_of_bound_backward;
	vector<string> out_of_bound__forward_par_vec;
	vector<string> out_of_bound__backard_par_vec;
	string tmp_name;

	// perturb derivative parameters
	double incr = derivative_inc(par_name, group_info, base_derivative_val, false);
	new_par_val = new_par[par_name] = base_derivative_val + incr;
	// try forward derivative
	out_of_bound_forward = out_of_bounds(new_par, par_info_ptr);
	if (!out_of_bound_forward) {
		return true;
	}
	// try backward derivative if forward derivative didn't work
	new_par.clear();
	new_par_val = new_par[par_name] = base_derivative_val - incr;
	out_of_bound_backward = out_of_bounds(new_par, par_info_ptr);
	if (!out_of_bound_backward)
	{	
		return true;
	}
	return false;
}

bool Jacobian_1to1::central_diff(const string &par_name, double base_derivative_val, 
		const ParameterGroupInfo &group_info, const ParameterInfo &ctl_par_info, const ParamTransformSeq &par_trans, vector<double> &new_par_vec, 
		vector<Parameters>  &perturb_derivative_par_vec)
{
	double new_par;
	bool out_of_bnds_forward, out_of_bnds_back, out_of_bnds;
	Parameters perturb_derivative_pars;
	string tmp_name;

	const ParameterRec *par_info_ptr = ctl_par_info.get_parameter_rec_ptr(par_name);
	double incr = derivative_inc(par_name, group_info, base_derivative_val, true);
	// try backward difference
	new_par = perturb_derivative_pars[par_name] = base_derivative_val - incr;
	out_of_bnds_back = out_of_bounds(perturb_derivative_pars, par_info_ptr);

	if (!out_of_bnds_back) {
		new_par_vec.push_back(new_par);
		perturb_derivative_par_vec.push_back(perturb_derivative_pars);
	}
	// try forward derivative
	new_par = perturb_derivative_pars[par_name] = base_derivative_val + incr;
	out_of_bnds_forward = out_of_bounds(perturb_derivative_pars, par_info_ptr);
	if (!out_of_bnds_forward) {
		new_par_vec.push_back(new_par);
		perturb_derivative_par_vec.push_back(perturb_derivative_pars);
	}
	// if backward difference was out of bounds do a second forward derivative
	if (out_of_bnds_back) {
		new_par = perturb_derivative_pars[par_name] = base_derivative_val + 2.0 * incr;
		out_of_bnds = out_of_bounds(perturb_derivative_pars, par_info_ptr);
		if (!out_of_bnds) {
			new_par_vec.push_back(new_par);
			perturb_derivative_par_vec.push_back(perturb_derivative_pars);
		}
		else
		{
			return false;  // can't do central difference without going out of bounds
		}
	}
	// if forward difference was out of bounds do a second backward derivative
	if (out_of_bnds_forward) {
		new_par = perturb_derivative_pars[par_name] = base_derivative_val - 2.0 * incr;
		out_of_bnds = out_of_bounds(perturb_derivative_pars, par_info_ptr);
		if (!out_of_bnds) {
			new_par_vec.insert(new_par_vec.begin(), new_par);
			perturb_derivative_par_vec.push_back(perturb_derivative_pars);
		}
		else
		{
			return false;  // can't do central difference without going out of bounds
		}
	}
	return true;
}

bool Jacobian_1to1::out_of_bounds(const Parameters &ctl_parameters,
	const ParameterRec *par_info_ptr) const
{
	bool out_of_bounds=false;

        // This will always only contain one entry one 1 to 1 Jacobians
	for (auto &p : ctl_parameters)
	{
		double max = par_info_ptr->ubnd;
		double min = par_info_ptr->lbnd;
		if (p.second > max || p.second < min) {
			out_of_bounds = true;
		}
	}
	return out_of_bounds;
}
