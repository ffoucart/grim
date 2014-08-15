#include "timestepper.h"

void diagnostics(struct timeStepper ts[ARRAY_ARGS 1])
{
  if (ts->timeStepCounter==0)
  {
    /* Dump geometry data */

    DM metricDMDA, connectionDMDA;
    Vec gCovPetscVec, gConPetscVec, connectionPetscVec;

#if (COMPUTE_DIM==1)
    DMDACreate1d(PETSC_COMM_WORLD, DM_BOUNDARY_NONE, N1, 16, 0, NULL,
                 &metricDMDA);
    DMDACreate1d(PETSC_COMM_WORLD, DM_BOUNDARY_NONE, N1, 64, 0, NULL,
                 &connectionDMDA);
#elif (COMPUTE_DIM==2)
    DMDACreate2d(PETSC_COMM_WORLD, DM_BOUNDARY_NONE, DM_BOUNDARY_NONE,
                 DMDA_STENCIL_BOX, N1, N2, PETSC_DECIDE, PETSC_DECIDE,
                 16, 0, PETSC_NULL, PETSC_NULL, &metricDMDA);
    DMDACreate2d(PETSC_COMM_WORLD, DM_BOUNDARY_NONE, DM_BOUNDARY_NONE,
                 DMDA_STENCIL_BOX, N1, N2, PETSC_DECIDE, PETSC_DECIDE,
                 64, 0, PETSC_NULL, PETSC_NULL, &connectionDMDA);
#endif /* Choose dimension */

    DMCreateGlobalVector(metricDMDA, &gCovPetscVec);
    DMCreateGlobalVector(metricDMDA, &gConPetscVec);
    DMCreateGlobalVector(connectionDMDA, &connectionPetscVec);

    REAL *gCovGlobal, *gConGlobal, *connectionGlobal;
    DMDAVecGetArray(metricDMDA, gCovPetscVec, &gCovGlobal);
    DMDAVecGetArray(metricDMDA, gConPetscVec, &gConGlobal);
    DMDAVecGetArray(metricDMDA, connectionPetscVec, &connectionGlobal);

    LOOP_OVER_TILES(ts->X2Size, ts->X1Size)
    {
      LOOP_INSIDE_TILE(0, TILE_SIZE_X2, 0, TILE_SIZE_X1)
      {
        
        struct gridZone zone;
        setGridZone(iTile, jTile,
                    iInTile, jInTile,
                    ts->X1Start, ts->X2Start,
                    ts->X1Size, ts->X2Size,
                    &zone);

        REAL XCoords[NDIM];
        getXCoords(&zone, CENTER, XCoords);

        struct geometry geom;
        setGeometry(XCoords, &geom);

        for (int mu=0; mu<NDIM; mu++)
        {
          for (int nu=0; nu<NDIM; nu++)
          {
            gCovGlobal[INDEX_METRIC_GLOBAL(&zone, mu, nu)] = geom.gCov[mu][nu];
            gCovGlobal[INDEX_METRIC_GLOBAL(&zone, mu, nu)] = geom.gCov[mu][nu];
          }
        }

        for (int eta=0; eta<NDIM; eta++)
        {
          for (int mu=0; mu<NDIM; mu++)
          {
            for (int nu=0; nu<NDIM; nu++)
            {
              connectionGlobal[INDEX_GAMMA_GLOBAL(&zone, eta, mu, nu)] =
                gammaDownDownDown(eta, mu, nu, XCoords);
            }
          }
        }

      }
    }

    DMDAVecRestoreArray(metricDMDA, gCovPetscVec, &gCovGlobal);
    DMDAVecRestoreArray(metricDMDA, gConPetscVec, &gConGlobal);
    DMDAVecRestoreArray(metricDMDA, connectionPetscVec, &connectionGlobal);

    char gCovFileName[50], gConFileName[50], connectionFileName[50];
    sprintf(gCovFileName, "gcov.h5");
    sprintf(gConFileName, "gcon.h5");
    sprintf(connectionFileName, "gammadowndowndown.h5");

    PetscViewer gCovViewer, gConViewer, connectionViewer;
    
    PetscViewerHDF5Open(PETSC_COMM_WORLD, gCovFileName,
                        FILE_MODE_WRITE, &gCovViewer);

    PetscViewerHDF5Open(PETSC_COMM_WORLD, gConFileName,
                        FILE_MODE_WRITE, &gConViewer);

    PetscViewerHDF5Open(PETSC_COMM_WORLD, connectionFileName,
                        FILE_MODE_WRITE, &connectionViewer);

    PetscObjectSetName((PetscObject) gCovPetscVec, "gCov");
    PetscObjectSetName((PetscObject) gConPetscVec, "gCov");
    PetscObjectSetName((PetscObject) connectionPetscVec,
                       "gammadowndowndown");

    VecView(gCovPetscVec, gCovViewer);
    VecView(gConPetscVec, gConViewer);
    VecView(connectionPetscVec, connectionViewer);

    PetscViewerDestroy(&gCovViewer);
    PetscViewerDestroy(&gConViewer);
    PetscViewerDestroy(&connectionViewer);

    VecDestroy(&gCovPetscVec);
    VecDestroy(&gConPetscVec);
    VecDestroy(&connectionPetscVec);

    DMDestroy(&metricDMDA);
    DMDestroy(&connectionDMDA);
  }

  if (ts->t >= ts->tDump)
  {
    PetscPrintf(PETSC_COMM_WORLD, "\n");
    PetscPrintf(PETSC_COMM_WORLD, "Dumping data\n");
    PetscPrintf(PETSC_COMM_WORLD, "\n");
    
    char dumpFileName[50];
    sprintf(dumpFileName, "%s%04d.h5", DUMP_FILE_PREFIX, ts->timeStepCounter);

    PetscViewer viewer;
    PetscViewerHDF5Open(PETSC_COMM_WORLD, dumpFileName,
                        FILE_MODE_WRITE, &viewer);
    PetscObjectSetName((PetscObject) ts->primPetscVec, "primVars");
    VecView(ts->primPetscVec, viewer);
    PetscViewerDestroy(&viewer);

    ts->tDump += DT_DUMP;
  }
  
}

