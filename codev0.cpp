#include <bits/stdc++.h>
#include <omp.h>

using namespace std;

#define F first
#define S second

typedef vector <int> vi;
typedef map <vi, int> mvii;
typedef vector <vi> vvi;

const int tr = 5000;

int min_sup, min_conf, max_items;
vvi transactions;
mvii itemsets;
vector <pair <vi, vi> > assoc_rules;

void get_transactions (string file_name) {
	freopen (file_name.c_str(), "r", stdin);
	vi ct;
	int p; while (cin >> p) {
			if (p == -1) transactions.emplace_back (ct), ct.clear();
			else ct.emplace_back (p);
	}
}

void generate_1_itemset () {
	map <int, int> tmp;
	int i;
	#pragma omp share (i) for collapse(2)
	for (i = 0; i < (int) transactions.size(); i++)
	for (int j = 0; j < (int) transactions[i].size(); j++) tmp[transactions[i][j]]++;
	omp_set_dynamic(1);
 	#pragma omp parallel num_threads(4)
 	{
	for (auto i : tmp) if (i.S >= min_sup) itemsets[{i.F}] = i.S;
	}
}

bool present (vi a, vi b) {
	int j = 0;
	for (auto i : a) if (i == b[j]) {
		j++;
		if (j == b.size()) return true;
	}
	return false;
}

void check_sup_count (vi arr) {
	int cnt = 0;
	#pragma omp share (cnt) for
	for (int i = 0; i < (int) transactions.size(); i++) {
		if (present (transactions[i], arr)) cnt++;
	}
	if (cnt >= min_sup) itemsets[arr] = cnt;
}

void prune (vvi nset, int sz) {
	for (auto i : nset) {
		bool ok = true;
		for (int j = 0; j < sz; j++) {
			vi dk;
			for (int k = 0; k < sz; k++) {
				if (k == j) continue;
				dk.emplace_back (i[k]);
			}
			if (itemsets.count(dk) == 0) {
				ok = false;
				break;
			}
		}
		if (ok) check_sup_count (i);
	}
}

void generate_itemset (int items) {
	vvi pset, nset;
	for (auto i : itemsets) if (i.F.size() == items - 1) pset.emplace_back (i.F);
	if (pset.empty()) {
		max_items = items - 2;
		return;
	}
	int i;
	#pragma omp share (i) for collapse(2)
	for (i = 0; i < pset.size (); i++) {
		for (int j = i + 1; j < pset.size(); j++) {
			set <int> ps;
			#pragma omp share (ps) parallel
			{
			#pragma omp for nowait
			for (int k = 0; k < (int) pset[i].size(); k++) ps.insert (pset[i][k]);
			#pragma omp for nowait
			for (int k = 0; k < (int) pset[j].size(); k++) ps.insert (pset[j][k]);
		  }
			if (ps.size() == items) {
				vi tk (ps.begin(), ps.end());
				nset.emplace_back (tk);
			}
		}
	}
	prune (nset, items);
	generate_itemset (items + 1);
}

void generate_association_rules () {
	for (auto i : itemsets) if (i.F.size() == max_items) {
		  #pragma omp for
			for (int j = 1; j < (1 << max_items) - 1; j++) {
				vi sset, tset;
				for (int k = 0; k < max_items; k++) {
					if (j & (1 << k)) sset.emplace_back (i.F[k]);
					else tset.emplace_back (i.F[k]);
				}
				double centage = (100.0 * i.S) / itemsets[sset];
				if (centage >= min_conf) assoc_rules.push_back (make_pair (sset, tset));
			}
	}
}

int main (int argc, char const *argv[]) {
	min_sup = atoi (argv[1]);			// min support count (number)
	min_conf = atoi (argv[2]);		// min confidence (percentage (integer))
	string file_name = argv[3];   //filename
	double start;
	start = omp_get_wtime();
	get_transactions (file_name);
	generate_1_itemset ();
	generate_itemset (2);
	/*cout << "Size of largest itemset = " << max_items << '\n';
	for (auto i : itemsets) {
		for (auto j : i.F) cout << j << " "; cout << "=> " << i.S << '\n';
	} */
	generate_association_rules ();
	cout << "Assiciation Rules : \n";
	for (auto i : assoc_rules) {
		for (int j = 0; j < i.F.size(); j++) {
			if (j) cout << " ^ ";
			cout << i.F[j];
		}
		cout << " ==> ";
		for (int j = 0; j < i.S.size(); j++) {
			if (j) cout << " ^ ";
			cout << i.S[j];
		}
		cout << '\n';
	}
	cout << "Time Taken : " << fixed << setprecision (8) << (omp_get_wtime() - start) << "\n";
	return 0;
}
