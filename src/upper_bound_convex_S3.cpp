/* Copyright (c) 2015, Julian Straub <jstraub@csail.mit.edu> Licensed
 * under the MIT license. See the license file LICENSE.
 */

#include "optRot/upper_bound_convex_S3.h"

namespace OptRot {

UpperBoundConvexS3::UpperBoundConvexS3(const vMFMM<3>&
    vmf_mm_A, const vMFMM<3>& vmf_mm_B) 
  : vmf_mm_A_(vmf_mm_A), vmf_mm_B_(vmf_mm_B)
{}

Eigen::Matrix<double,4,4> BuildM(const Eigen::Vector3d& u, const
    Eigen::Vector3d& v) {
  const double ui = u(0);
  const double uj = u(1);
  const double uk = u(2);
  const double vi = v(0);
  const double vj = v(1);
  const double vk = v(2);
  Eigen::Matrix<double,4,4> M;
  M << u.transpose()*v, uk*vj-uj*vk,       ui*vk-uk*vi,       uj*vi-ui*vj, 
       uk*vj-uj*vk,     ui*vi-uj*vj-uk*vk, uj*vi+ui*vj,       ui*vk+uk*vi,
       ui*vk-uk*vi,     uj*vi+ui*vj,       uj*vj-ui*vi-uk*vk, uj*vk+uk*vj,
       uj*vi-ui*vj,     ui*vk+uk*vi,       uj*vk+uk*vj,       uk*vk-ui*vi-uj*vj;
  return M;
}

double FindMaximumQAQ(const Eigen::Matrix4d& A, const Tetrahedron4D&
    tetrahedron) {
  std::vector<double> lambdas;
  Eigen::Matrix4d Q;
  for (uint32_t i=0; i<4; ++i)
    Q.col(i) = tetrahedron.GetVertex(i);
  // Only one q: 
  for (uint32_t i=0; i<4; ++i)
    lambdas.push_back(Q.col(i).transpose() * A * Q.col(i));
  // Full problem:
  Eigen::Matrix4d A_ = Q.transpose() * A * Q;
  Eigen::Matrix4d B_ = Q.transpose() * Q;
  double lambda = 0.;
  if (FindLambda<4>(A_,B_, &lambda))
    lambdas.push_back(lambda);
  // Only three or two qs: 
  Combinations comb43s(4,3);
  for (auto comb : comb43s.Get()) {
    Eigen::Matrix3d A__; 
    Eigen::Matrix3d B__;
    for (uint32_t i=0; i<3; ++i)
      for (uint32_t j=0; j<3; ++j) {
        A__(i,j) = A_(comb[i],comb[j]);
        B__(i,j) = B_(comb[i],comb[j]);
      }
    if (FindLambda<3>(A__, B__, &lambda))
      lambdas.push_back(lambda);
  }
  Combinations comb42s(4,2);
  for (auto comb : comb42s.Get()) {
    Eigen::Matrix2d A__; 
    Eigen::Matrix2d B__;
    for (uint32_t i=0; i<2; ++i)
      for (uint32_t j=0; j<2; ++j) {
        A__(i,j) = A_(comb[i],comb[j]);
        B__(i,j) = B_(comb[i],comb[j]);
      }
    if (FindLambda<2>(A__, B__, &lambda))
      lambdas.push_back(lambda);
  }
//  std::cout << "Q" << std::endl << Q << std::endl;
//  for (auto l : lambdas) 
//    std::cout << l << " ";
//  std::cout << std::endl;
  return *std::max_element(lambdas.begin(), lambdas.end());
}

double UpperBoundConvexS3::Evaluate(const NodeS3& node) {

  std::vector<Eigen::Matrix4d> Melem(vmf_mm_A_.GetK()*vmf_mm_B_.GetK());
  Eigen::VectorXd Aelem(vmf_mm_A_.GetK()*vmf_mm_B_.GetK());
  Eigen::MatrixXd Belem(vmf_mm_A_.GetK()*vmf_mm_B_.GetK(),4);
  Eigen::MatrixXd BelemSign(vmf_mm_A_.GetK()*vmf_mm_B_.GetK(),4);

  for (std::size_t j=0; j < vmf_mm_A_.GetK(); ++j)
    for (std::size_t k=0; k < vmf_mm_B_.GetK(); ++k) {
      const vMF<3>& vmf_A = vmf_mm_A_.Get(j);
      const vMF<3>& vmf_B = vmf_mm_B_.Get(k);
      Eigen::Vector3d p_U = ClosestPointInTetrahedron(vmf_A,
          vmf_B, node.GetTetrahedron());
      Eigen::Vector3d p_L = FurthestPointInTetrahedron(vmf_A,
          vmf_B, node.GetTetrahedron());
  //    std::cout << p_U.transpose() << " and " << p_L.transpose() << std::endl;
      double U = (vmf_A.GetTau()*vmf_A.GetMu() +
          vmf_B.GetTau()*p_U).norm();
      double L = (vmf_A.GetTau()*vmf_A.GetMu() +
          vmf_B.GetTau()*p_L).norm();
      double LfU = 0.;
      double UfL = 0.;
      double fUfLoU2L2 = 0.;
      double L2fUU2fLoU2L2 = 0.;
      std::cout << "-- U " << U << " L " << L << std::endl;
      if (fabs(U-L) < 1.e-6) {
        if (U > 50.) {
          fUfLoU2L2 = log(U-1.) + 2.*U - log(2.) - 3.*log(U) - U;
          LfU = log(U) + 2.*U - log(2.) - log(U) - U;
        } else {
          LfU = log(3+U+U*exp(2.*U)) - log(2.) - log(U) - U;
          fUfLoU2L2 = log(1. + U + (U-1.) * exp(2.*U)) - log(2.) - 3.*log(U) - U;
        }
        UfL = log(3.) + 2.*U - log(2.) - log(U) - U;
      } else {
        double f_U = ComputeLog2SinhOverZ(U);
        double f_L = ComputeLog2SinhOverZ(L);
        std::cout << "f_U " << f_U << " f_L " << f_L << std::endl;
        fUfLoU2L2 = - log(U - L) - log(U + L);
        if (f_U > f_L) {
          fUfLoU2L2 += log(1. - exp(f_L-f_U)) + f_U;
          std::cout << "f_L - f_U " << f_L-f_U << " exp(.) " << exp(f_L-f_U)
            << std::endl;
        } else {
          fUfLoU2L2 += log(exp(f_U-f_L) - 1.) + f_L;
          std::cout << "f_U - f_L " << f_U-f_L << " exp(.) " << exp(f_U-f_L)
            << std::endl;
        }
        L2fUU2fLoU2L2 = -log(U - L) -log(U + L);
        LfU = 2.*log(L)+f_U + L2fUU2fLoU2L2;
        UfL = 2.*log(U)+f_L + L2fUU2fLoU2L2;
        std::cout << "LfU " << LfU << " UfL " << UfL << std::endl;
      }
      uint32_t K = vmf_mm_B_.GetK();
      Melem[j*K+k] = BuildM(vmf_A.GetMu(), vmf_B.GetMu());
//      std::cout << vmf_A.GetMu().transpose() << " and " << vmf_B.GetMu().transpose() << std::endl;
//      std::cout << j << " k " << k << std::endl << Melem[j*k+k] << std::endl;
      double D = log(2.*M_PI) + log(vmf_A.GetPi()) + log(vmf_B.GetPi())
        + vmf_A.GetLogZ() + vmf_B.GetLogZ();
      Aelem(j*K+k) = log(2) + log(vmf_A.GetTau()) + log(vmf_B.GetTau())
        + D + fUfLoU2L2;
      Eigen::Vector4d b;
      b << 2.*log(vmf_A.GetTau()) + fUfLoU2L2,
        2.*log(vmf_B.GetTau())+fUfLoU2L2, LfU, UfL;
      Belem.row(j*K+k) = (b.array()+D).matrix();
      BelemSign.row(j*K+k) << 1.,1.,-1.,1.;
    }
  Eigen::Matrix4d A;
  for (uint32_t j=0; j<4; ++j)
    for (uint32_t k=0; k<4; ++k) {
      Eigen::VectorXd M_jk_elem(Melem.size());
      for (uint32_t i=0; i<Melem.size(); ++i)
        M_jk_elem(i) = Melem[i](j,k);
//        std::cout << j << " " << k << " " << M_jk_elem.transpose() << std::endl;
      A(j,k) = (M_jk_elem.array()*(Aelem.array() -
            Aelem.maxCoeff()).array().exp()).sum() *
        exp(Aelem.maxCoeff());
    }
//  std::cout << "Aelem " << Aelem.transpose() << std::endl;
  double B = (BelemSign.array()*(Belem.array() -
        Belem.maxCoeff()).exp()).sum() * exp(Belem.maxCoeff());
  std::cout << "A " <<  std::endl;
  std::cout << A << std::endl;
  double lambda_max = FindMaximumQAQ(A, node.GetTetrahedron());
  std::cout << "B " << B << " lambda_max " << lambda_max << std::endl;
  return B + lambda_max;
}

double UpperBoundConvexS3::EvaluateAndSet(NodeS3& node) {
  double ub = Evaluate(node);
  node.SetUB(ub);
  return ub;
}

}
