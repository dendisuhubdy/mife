#include <gghlite/gghlite.h>
#include <gghlite/gghlite-internals.h>
#include <oz/oz.h>

int test_inv_approx(long n, long prec) {
  flint_rand_t randstate;
  flint_randinit_seed(randstate, 0x1337, 1);

  fmpz_poly_t f;
  fmpz_poly_init(f);

  fmpq_poly_t modulus_q; fmpq_poly_init(modulus_q);
  fmpq_poly_set_coeff_si(modulus_q, 0, 1);
  fmpq_poly_set_coeff_si(modulus_q, n, 1);
  fmpz_poly_t modulus_z; fmpz_poly_init(modulus_z);
  fmpz_poly_set_coeff_si(modulus_z, 0, 1);
  fmpz_poly_set_coeff_si(modulus_z, n, 1);

  mpfr_t sigma;
  mpfr_init2(sigma, prec);
  mpfr_set_d(sigma, _gghlite_sigma(n), MPFR_RNDN);

  fmpz_poly_sample_sigma(f, n, sigma, randstate);
  printf("log(sigma): %lf\n",log2(mpfr_get_d(sigma, MPFR_RNDN)));

  fmpq_poly_t f_q; fmpq_poly_init(f_q);
  fmpq_poly_set_fmpz_poly(f_q, f);
  fmpq_poly_t f_inv1; fmpq_poly_init(f_inv1);
  uint64_t t1 = ggh_walltime(0);
  _fmpq_poly_oz_invert_approx(f_inv1, f_q, n, prec);
  t1 = ggh_walltime(t1);
  printf("prec %ld t: %7.1f\n", prec, t1/1000000.0);

  fmpq_poly_t f_inv2; fmpq_poly_init(f_inv2);
  uint64_t t2 = ggh_walltime(0);
  fmpq_poly_oz_invert_approx(f_inv2, f_q, n, prec, OZ_VERBOSE);
  t2 = ggh_walltime(t2);
  printf("iter prec %ld t: %7.1f\n", prec, t2/1000000.0);

  fmpq_poly_t f_inv3; fmpq_poly_init(f_inv3);
  uint64_t t3 = ggh_walltime(0);
  _fmpq_poly_oz_invert_approx(f_inv3, f_q, n, 0);
  t3 = ggh_walltime(t3);
  printf("CATALIN t: %7.1f\n", t3/1000000.0);

  fmpq_poly_t f_inv0; fmpq_poly_init(f_inv0);
  uint64_t t0 = ggh_walltime(0);
  fmpz_poly_invert_mod_fmpq(f_inv0, f, modulus_z);
  t0 = ggh_walltime(t0);
  printf("  FLINT t: %7.1f\n", t0/1000000.0);
  return 0;
}

int test_sqrt_approx(long n, long prec) {
  flint_rand_t randstate;
  flint_randinit_seed(randstate, 0x1337, 1);

  fmpz_poly_t f;
  fmpz_poly_init(f);

  mpfr_t sigma;
  mpfr_init2(sigma, prec);
  mpfr_set_d(sigma, _gghlite_sigma(n), MPFR_RNDN);

  fmpz_poly_sample_sigma(f, n, sigma, randstate);

  fmpq_poly_t Sigma_sqrt;
  fmpq_poly_init(Sigma_sqrt);

  _dgsl_rot_mp_sqrt_sigma_2(Sigma_sqrt, f, sigma, ceil(2*log2(n)), n, prec, OZ_VERBOSE);
  return 0;
}



int main(int argc, char *argv[]) {
  assert(argc>=3);
  const long n    = atol(argv[1]);
  const long prec = atol(argv[2]);

  flint_rand_t randstate;
  flint_randinit_seed(randstate, 0x1337, 1);

  test_inv_approx(n, prec);
  
  fmpz_poly_t g;
  fmpz_poly_init(g);

  mpfr_t sigma;
  mpfr_init2(sigma, 80);
  mpfr_set_d(sigma, _gghlite_sigma(n), MPFR_RNDN);

  fmpz_poly_t modulus;
  fmpz_poly_oz_init_modulus(modulus, n);

  uint64_t t = ggh_walltime(0);

  int r = 0;
  for(size_t i=0; i<prec; i++) {
    fmpz_poly_sample_sigma(g, n, sigma, randstate);
    r += fmpz_poly_ideal_is_probaprime(g, modulus, 0);
    printf("%d ",r); fflush(0);
  }
  printf("\n");

  t = ggh_walltime(t);
  printf("t: %.6f\n",t/1000000.0/prec);
  
  mpfr_clear(sigma);
  fmpz_poly_clear(g);
  flint_randclear(randstate);

  return 0;
}
