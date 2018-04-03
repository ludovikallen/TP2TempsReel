#include <new>
#include <functional>
#include <vector>
#include <random>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <utility>
#include <memory>
#include <cassert>
#include <numeric>
#include <bitset>
using namespace std;
using namespace std::chrono;

namespace kh2 {
	namespace v0 {
		//
		// version archi naïve, qui ne respecte pas les consignes,
		// écrite juste pour que le code de test compile
		//
		class GestionnaireBlocs {
		public:
			void* pPetit = malloc(32 * 1'000'000);
			void* pMoyen = malloc(64 * 500'000);
			void* pGros = malloc(128 * 250'000);

			vector<int> vPetit;
			vector<int> vMoyen;
			vector<int> vGros;
			void init()
			{
				for (size_t i = 0; i < 1'000'000; i++)
				{
					vPetit.push_back((int)pPetit + i * 32);
				}
				for (size_t i = 0; i < 500'000; i++)
				{
					vMoyen.push_back((int)pMoyen + i * 64);
				}
				for (size_t i = 0; i < 250'000; i++)
				{
					vGros.push_back((int)pGros + i * 128);
				}
			}
			/*
			GestionnaireBlocs() :vPetit(1000000, false), vMoyen(500000, false), vGros(250000, false) {}*/
			GestionnaireBlocs() = default;
			GestionnaireBlocs(const GestionnaireBlocs&) = delete;
			GestionnaireBlocs& operator=(const GestionnaireBlocs&) = delete;
			void * allouer(size_t n) {
				if (n <= 32) {
					auto p = vPetit.back();
					vPetit.pop_back();
					auto q = reinterpret_cast<size_t*> (p);
					*q = n;
					return q;
				}
				else if (n <= 64) {
					auto p = vMoyen.back();
					vMoyen.pop_back();
					auto q = reinterpret_cast<size_t*> (p);
					*q = n;
					return q;
				}
				else if (n <= 128) {
					auto p = vGros.back();
					vGros.pop_back();
					auto q = reinterpret_cast<size_t*> (p);
					*q = n;
					return q;
				}
				else {
					return ::operator new(n);
				}
			}
			void liberer(void *p) {
				if (!p) return;
				if ((int)p >= (int)pPetit && (int)p <= (int)pPetit + 32 * 1'000'000)
				{
					vPetit.push_back((int)p);
					memset(p, 10, 32);
				}
				else if ((int)p >= (int)pMoyen && (int)p <= (int)pMoyen + 32 * 1'000'000)
				{
					vMoyen.push_back((int)p);
					memset(p, 10, 64);
				}
				else if ((int)p >= (int)pGros && (int)p <= (int)pGros + 32 * 1'000'000)
				{
					vGros.push_back((int)p);
					memset(p, 10, 128);
				}
				else {
					operator delete(p);
				}
			}
			//
			// pour afficher les statistiques
			//
			friend ostream& operator<<(ostream &os, const GestionnaireBlocs &) {
				return os << "Version 0";
			}
		};
	}
}

using kh2::v0::GestionnaireBlocs;

void * operator new(size_t n, GestionnaireBlocs &gb) {
	return gb.allouer(n);
}
void operator delete(void *p, GestionnaireBlocs &gb) {
	return gb.liberer(p);
}

template <int N>
struct moton {
	char _[N]{};
};

//template <int N>
//void assassin(void *p) {
//   static_cast<moton<N>*>(p)->~moton<N>();
//}
//
template <int N>
pair<void*, function<void(void*)>> creer_moton(GestionnaireBlocs &gb) {
	return pair<void*, function<void(void*)>>{
		new (gb) moton<N>,
			[&gb](void *p) {
			static_cast<moton<N>*>(p)->~moton<N>();
			gb.liberer(p);
		}
	};
}

int main() {
	GestionnaireBlocs gb;
	int tres_petits = 0;
	int petits = 0;
	int pas_gros = 0;
	int autres = 0;
	gb.init();
	vector<function<pair<void*, function<void(void*)>>(GestionnaireBlocs&)>> ges = {
		[&](GestionnaireBlocs &gb) {
		++tres_petits;
		return creer_moton<1>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++tres_petits;
		return creer_moton<2>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++tres_petits;
		return creer_moton<4>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++tres_petits;
		return creer_moton<7>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++tres_petits;
		return creer_moton<8>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++tres_petits;
		return creer_moton<31>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++tres_petits;
		return creer_moton<32>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++petits;
		return creer_moton<33>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++petits;
		return creer_moton<50>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++petits;
		return creer_moton<63>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++petits;
		return creer_moton<64>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++pas_gros;
		return creer_moton<65>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++pas_gros;
		return creer_moton<127>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++pas_gros;
		return creer_moton<128>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++autres;
		return creer_moton<129>(gb);
	},
		[&](GestionnaireBlocs &gb) {
		++autres;
		return creer_moton<200>(gb);
	}
	};
	enum { N = 1'000'000, NTESTS = 100 };
	vector<void *> ptrs;
	vector<function<void(void*)>> deleters;
	ptrs.reserve(N);
	deleters.reserve(N);
	// random_device rd;
	mt19937 prng{ 0 /* rd() */ };
	uniform_int_distribution<decltype(ges.size())> de{ 0, ges.size() - 1 };
	vector<high_resolution_clock::duration> temps_par_test;
	for (auto t = 0; t != NTESTS; ++t) {
		auto avant = high_resolution_clock::now();
		for (int i = 0; i != N; ++i) {
			auto pos = de(prng);
			auto[ptr, del] = ges[pos](gb);
			ptrs.push_back(ptr);
			deleters.push_back(del);
		}
		for (vector<void*>::size_type i = 0; i != ptrs.size(); ++i) {
			deleters[i](ptrs[i]);
		}
		auto apres = high_resolution_clock::now();
		ptrs.clear();
		deleters.clear();
		temps_par_test.emplace_back(apres - avant);
		cout << '.' << flush;
	}
	auto temps_total = accumulate(
		begin(temps_par_test), end(temps_par_test), high_resolution_clock::duration{}
	);
	if (tres_petits + petits + pas_gros + autres != N * NTESTS)
		cerr << "Erreur suspecte de calcul (ceci ne devrait pas s'afficher)" << endl;
	cout << "\n\nNombre de tests : " << NTESTS
		<< "\nNombre d'allocations par test : " << N
		<< "\nNombre de [0,32] bytes) : " << tres_petits
		<< "\nNombre de (32,64] bytes : " << petits
		<< "\nNombre de (64,128] bytes : " << pas_gros
		<< "\nNombre de (128+ bytes : " << autres
		<< "\nTemps ecoule (total) : " << duration_cast<milliseconds>(temps_total).count() << " ms."
		<< "\nTemps ecoule (moyen par test) : "
		<< static_cast<double>(duration_cast<milliseconds>(temps_total).count()) / NTESTS << " ms." << endl;
	cout << gb << endl;
}