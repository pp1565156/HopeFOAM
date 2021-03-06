/*--------------------------------------------------------------------------------------
|     __  ______  ____  ______ |                                                       |
|    / / / / __ \/ __ \/ ____/ | HopeFOAM: High Order Parallel Extensible CFD Software |
|   / /_/ / / / / /_/ / __/    |                                                       |
|  / __  / /_/ / ____/ /___    |                                                       |
| /_/ /_/\____/_/   /_____/    | Copyright(c) 2017-2017 The Exercise Group, AMS, China.|
|                              |                                                       |
----------------------------------------------------------------------------------------

License
    This file is part of HopeFOAM, which is developed by Exercise Group, Innovation 
    Institute for Defence Science and Technology, the Academy of Military Science (AMS), China.

    HopeFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    HopeFOAM is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with HopeFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "dgForceCoeffs.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{
    defineTypeNameAndDebug(dgForceCoeffs, 0);
    addToRunTimeSelectionTable(functionObject, dgForceCoeffs, dictionary);
}
}


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void Foam::functionObjects::dgForceCoeffs::writeFileHeader(const label i)
{
    if (i == 0)
    {
        // force coeff data

        writeHeader(file(i), "dgForce coefficients");
        writeHeaderValue(file(i), "liftDir", liftDir_);
        writeHeaderValue(file(i), "dragDir", dragDir_);
        writeHeaderValue(file(i), "pitchAxis", pitchAxis_);
        writeHeaderValue(file(i), "magUInf", magUInf_);
        writeHeaderValue(file(i), "lRef", lRef_);
        writeHeaderValue(file(i), "Aref", Aref_);
        writeHeaderValue(file(i), "CofR", coordSys_.origin());
        writeCommented(file(i), "Time");
        writeTabbed(file(i), "Cm");
        writeTabbed(file(i), "Cd");
        writeTabbed(file(i), "Cl");
        writeTabbed(file(i), "Cl(f)");
        writeTabbed(file(i), "Cl(r)");
        file(i)
            << tab << "Cm" << tab << "Cd" << tab << "Cl" << tab << "Cl(f)"
            << tab << "Cl(r)";
    }
    else if (i == 1)
    {
        // bin coeff data

        writeHeader(file(i), "dgForce coefficient bins");
        writeHeaderValue(file(i), "bins", nBin_);
        writeHeaderValue(file(i), "start", binMin_);
        writeHeaderValue(file(i), "delta", binDx_);
        writeHeaderValue(file(i), "direction", binDir_);

        vectorField binPoints(nBin_);
        writeCommented(file(i), "x co-ords  :");
        forAll(binPoints, pointi)
        {
            binPoints[pointi] = (binMin_ + (pointi + 1)*binDx_)*binDir_;
            file(i) << tab << binPoints[pointi].x();
        }
        file(i) << nl;

        writeCommented(file(i), "y co-ords  :");
        forAll(binPoints, pointi)
        {
            file(i) << tab << binPoints[pointi].y();
        }
        file(i) << nl;

        writeCommented(file(i), "z co-ords  :");
        forAll(binPoints, pointi)
        {
            file(i) << tab << binPoints[pointi].z();
        }
        file(i) << nl;

        writeCommented(file(i), "Time");

        for (label j = 0; j < nBin_; j++)
        {
            const word jn('(' + Foam::name(j) + ')');
            writeTabbed(file(i), "Cm" + jn);
            writeTabbed(file(i), "Cd" + jn);
            writeTabbed(file(i), "Cl" + jn);
        }
    }
    else
    {
        FatalErrorInFunction
            << "Unhandled file index: " << i
            << abort(FatalError);
    }

    file(i)<< endl;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::functionObjects::dgForceCoeffs::dgForceCoeffs
(
    const word& name,
    const Time& runTime,
    const dictionary& dict
)
:
    dgForces(name, runTime, dict),
    liftDir_(Zero),
    dragDir_(Zero),
    pitchAxis_(Zero),
    magUInf_(0.0),
    lRef_(0.0),
    Aref_(0.0)
{
    read(dict);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::functionObjects::dgForceCoeffs::~dgForceCoeffs()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::functionObjects::dgForceCoeffs::read(const dictionary& dict)
{
    dgForces::read(dict);

    // Directions for lift and drag forces, and pitch moment
    dict.lookup("liftDir") >> liftDir_;
    dict.lookup("dragDir") >> dragDir_;
    dict.lookup("pitchAxis") >> pitchAxis_;

    // Free stream velocity magnitude
    dict.lookup("magUInf") >> magUInf_;

    // Reference length and area scales
    dict.lookup("lRef") >> lRef_;
    dict.lookup("Aref") >> Aref_;

    return true;
}


bool Foam::functionObjects::dgForceCoeffs::execute()
{
    return true;
}


bool Foam::functionObjects::dgForceCoeffs::write()
{
    dgForces::calcForcesMoment();

    if (Pstream::master())
    {
        writeFiles::write();

        scalar pDyn = 0.5*rhoRef_*magUInf_*magUInf_;

        Field<vector> totForce(force_[0] + force_[1] + force_[2]);
        Field<vector> totMoment(moment_[0] + moment_[1] + moment_[2]);

        List<Field<scalar>> coeffs(3);
        coeffs[0].setSize(nBin_);
        coeffs[1].setSize(nBin_);
        coeffs[2].setSize(nBin_);

        // lift, drag and moment
        coeffs[0] = (totForce & liftDir_)/(Aref_*pDyn);
        coeffs[1] = (totForce & dragDir_)/(Aref_*pDyn);
        coeffs[2] = (totMoment & pitchAxis_)/(Aref_*lRef_*pDyn);


        scalar Cl = sum(coeffs[0]);
        scalar Cd = sum(coeffs[1]);
        scalar Cm = sum(coeffs[2]);

        scalar Clf = Cl/2.0 + Cm;
        scalar Clr = Cl/2.0 - Cm;

        writeTime(file(0));
        file(0)
            << tab << Cm << tab  << Cd
            << tab << Cl << tab << Clf << tab << Clr << endl;

        Log << type() << " " << name() << " write:" << nl
            << "    Cm    = " << Cm << nl
            << "    Cd    = " << Cd << nl
            << "    Cl    = " << Cl << nl
            << "    Cl(f) = " << Clf << nl
            << "    Cl(r) = " << Clr << endl;

        if (nBin_ > 1)
        {
            if (binCumulative_)
            {
                for (label i = 1; i < coeffs[0].size(); i++)
                {
                    coeffs[0][i] += coeffs[0][i-1];
                    coeffs[1][i] += coeffs[1][i-1];
                    coeffs[2][i] += coeffs[2][i-1];
                }
            }

            writeTime(file(1));

            forAll(coeffs[0], i)
            {
                file(1)
                    << tab << coeffs[2][i]
                    << tab << coeffs[1][i]
                    << tab << coeffs[0][i];
            }

            file(1) << endl;
        }

        Log << endl;
    }

    return true;
}


// ************************************************************************* //
