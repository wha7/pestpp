        !COMPILER-GENERATED INTERFACE MODULE: Wed Jun 27 16:46:29 2012
        MODULE DMGS__genmod
          INTERFACE 
            SUBROUTINE DMGS(N,K,V,LDV,VNEW,INDEX)
              INTEGER(KIND=4) :: LDV
              INTEGER(KIND=4) :: N
              INTEGER(KIND=4) :: K
              REAL(KIND=8) :: V(LDV,*)
              REAL(KIND=8) :: VNEW(*)
              INTEGER(KIND=4) :: INDEX(*)
            END SUBROUTINE DMGS
          END INTERFACE 
        END MODULE DMGS__genmod
