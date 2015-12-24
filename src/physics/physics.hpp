#ifndef GRIM_PHYSICS_H_
#define GRIM_PHYSICS_H_

#include "../params.hpp"
#include "../grid/grid.hpp"
#include "../geometry/geometry.hpp"

inline int DELTA(int const &mu, int const &nu)
{
  return (mu==nu ? 1 : 0);
}

class fluidElement
{
  private:
    const std::vector<std::vector<int>> indicesToLoopOver 
      = {{0},    {0, 1}, {0, 1}, {0, 2}, {0, 2}, {0, 3}, {0, 3}};
    /*   CENTER, LEFT,   RIGHT,  TOP,    BOTTOM, FRONT,  BACK*/

    array one;
  public:
    int loc;

    /* fluidElement parameters */
    array tau, chi, nu;
    
    array rho, u, u1, u2, u3, B1, B2, B3;
    array pressure, temperature;
    array qTilde, deltaPTilde;
    array q, deltaP;
  
    array gammaLorentzFactor, uCon[NDIM], uCov[NDIM];
    array bSqr, bCon[NDIM], bCov[NDIM];
    
    array NUp[NDIM];
    array TUpDown[NDIM][NDIM];

    fluidElement(const grid &prim, 
                 const geometry &geom, 
                 const int location
                );
    void set(const grid &prim, 
             const geometry &geom, 
             const int location
            );
    void setFluidElementParameters(const geometry &geom);
    void computeFluxes(const geometry &geom, 
                       const int direction,
                       grid &flux
                      );
  // Source terms. We use elemOld to compute time derivatives,
  // and elemForSpatialDeriv to compute spatial derivatives.
  // The timestepper is responsible for providing the correct
  // elements for each operation, and the dt to use for time derivatives.
  // NOTE: We assume that the current element doesn't have ghost zones,
  // but that both elemOld and elemForSpatialDeriv do!!
    void computeSources(const geometry &geom,
			const fluidElement &elemOld,
			const fluidElement &elemForSpatialDeriv,
			const double dt,
			const int UseImplicitSources,
                        grid &sources
                       );
};

class riemannSolver
{
  public:
    fluidElement *elemLeft, *elemRight;

    grid *primLeft, *primRight;
    grid *fluxLeft, *fluxRight;
    grid *consLeft, *consRight;

    riemannSolver(const geometry &geom);
    ~riemannSolver();

    void reconstruct(const grid &prim,
                     const int dir,
                     grid &primLeft,
                     grid &primRight
                    );

  array slopeMM(const int dir,
	       const array& in);

  void reconstructWENO5(const grid &prim,
                     const int dir,
                     grid &primLeft,
                     grid &primRight
                    );

  void solve(const grid &prim,
               const geometry &geom,
               const int dir,
               grid &fluxes
              );
};

#endif /* GRIM_PHYSICS_H_ */