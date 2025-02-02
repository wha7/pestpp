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
#include "RunManagerSerial.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <iterator>
#include <cassert>
#include <cstring>
#include <map>
#include <algorithm>
#include "system_variables.h"
#include "Transformable.h"
#include "utilities.h"
#include "model_interface.h"

using namespace std;
using namespace pest_utils;


RunManagerSerial::RunManagerSerial(const vector<string> _comline_vec,
	const vector<string> _tplfile_vec, const vector<string> _inpfile_vec,
	const vector<string> _insfile_vec, const vector<string> _outfile_vec,
	const string &stor_filename, const string &_run_dir, int _max_run_fail)
	: RunManagerAbstract(_comline_vec, _tplfile_vec, _inpfile_vec,
	_insfile_vec, _outfile_vec, stor_filename, _max_run_fail),
	run_dir(_run_dir), mi(_tplfile_vec,_inpfile_vec,_insfile_vec,_outfile_vec, _comline_vec)
{
	
	cout << "              starting serial run manager ..." << endl << endl;
}

void RunManagerSerial::run()
{
	int success_runs = 0;
	int prev_sucess_runs = 0;
	const vector<string> &par_name_vec = file_stor.get_par_name_vec();
	const vector<string> &obs_name_vec = file_stor.get_obs_name_vec();
	
	stringstream message;		
	std::vector<double> obs_vec;
	vector<int> run_id_vec;
	int nruns = get_outstanding_run_ids().size();
	while (!(run_id_vec = get_outstanding_run_ids()).empty())
	{
		for (int i_run : run_id_vec)
		{						
			try 
			{
				Observations obs;
				vector<double> par_values;
				Parameters pars;
				file_stor.get_parameters(i_run, pars);
				obs_vec.resize(obs_name_vec.size(), RunStorage::no_data);
				obs.clear();
				obs.insert(obs_name_vec, obs_vec);
				mi.run(&pars, &obs);			
				std::cout << string(message.str().size(), '\b');
				message.str("");
				message << "(" << success_runs << "/" << nruns << " runs complete)";
				std::cout << message.str();
				OperSys::chdir(run_dir.c_str());
				success_runs += 1;
				file_stor.update_run(i_run, pars, obs);

			}
			catch (const std::exception& ex)
			{
				update_run_failed(i_run);
				cerr << endl;
				cerr << "  " << ex.what() << endl;
				cerr << "  Aborting model run" << endl << endl;
			}
			catch (...)
			{
				update_run_failed(i_run);
				cerr << endl;
				cerr << "  Error running model" << endl;
				cerr << "  Aborting model run" << endl << endl;
			}
		}
	}
	total_runs += success_runs;
	std::cout << string(message.str().size(), '\b');
	message.str("");
	message << "(" << success_runs << "/" << nruns << " runs complete";
	if (prev_sucess_runs > 0)
	{
		message << " and " << prev_sucess_runs << " additional run completed previously";
	}
	message << ")";
	std::cout << message.str();
	if (success_runs < nruns)
	{			cout << endl << endl;
		cout << "WARNING: " << nruns - success_runs << " out of " << nruns << " runs failed" << endl << endl;
	}
	std::cout << endl << endl;
	if (init_sim.size() == 0)
	{
		vector<double> pars;
		int status = file_stor.get_run(0, pars, init_sim);
	}
}


RunManagerSerial::~RunManagerSerial(void)
{
}
