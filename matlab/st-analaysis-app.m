classdef PCMC_Analysis < handle
% Analysis class for a 3-dimensional framed structure
    
    % Private properties go here
    properties (Access = private)
        nnodes              % Number of nodes
        nele                % Number of elements
        concen              % Nodal concentraded forces
        coord               % Coordinate matrix
        a_nodes             % a prefix for analysis class
        a_elements          % a prefix for analysis class
        fixity_transpose    % Fixity matrix of nodes, transposed
        
        % Matrices returned to Mastan2:
        AFLAG               % Returns to MASTAN2 if kff matrix is unstable
        REACT               % Returns support reaction forces to MASTAN2 
        DEFL                % Returns deflections to MASTAN2 
        ELE_FOR             % Returns element forces to MASTAN2 
    end
    
    % Public methods go here
    methods (Access = public)
        %% Constructor
        function self = PCMC_Analysis(nnodes,nele,coord,concen,fixity,ends,Axx,Ayy,Azz,Iyy,Izz,J,E,v,webdir,w)
            self.nnodes = nnodes;               % Number of nodes
            self.nele = nele;                   % Number of elements
            self.coord = coord;                 % Coordinate matrix
            self.concen = concen;               % Concentrated nodal forces matrix
            self.fixity_transpose = fixity';    % transpose of fixity matrix
            
            self.a_nodes = create_nodes(self);  % Vector with PCMC_Node classes
            self.a_elements = create_elements(self,nele,ends,Axx,Ayy,Azz,Iyy,Izz,J,E,v,webdir,w); % Vector with PCMC_Element classes
        end
        
        %% Run the analysis
        function RunAnalysis(self)
            stiffness_matrix = CreateGlobalStiffness(self);         % Create global stiffness matrix
            [dof_f,dof_n,dof_s] = ClassifyDOF(self);                % Classify degrees of freedom
            [kff, kfn, knf, knn, ksf, ksn] = ComputeStiffSubMat(self,stiffness_matrix,dof_f,dof_n,dof_s);       % Extract stiffness submatrices from global matrix
            [P_f,P_n,P_s,FEF_f,FEF_n,FEF_s] = CreateGlobalLoad(self,dof_f,dof_n,dof_s);                         % Create load vector in global
            [DEFL, d_n, d_f] = ComputeDispReact(self,kff,kfn,knf,knn,ksf,ksn,dof_f,dof_n,dof_s,P_f,P_n,P_s,FEF_f,FEF_n,FEF_s);  % Compute Displacements and Reactions
            CheckKFF(self,kff);                                     % Checks if kff unstable
            [ELE_FOR] = self.RecoverElemFor();                      % Recover element forces
            ComputeError(self,kff,kfn,d_f,d_n,P_f,FEF_f);           % Compute calculation errors
        end
             
        % Return matrices to MASTAN2
        function [AFLAG, REACT, DEFL, ELE_FOR] = GetMastan2Returns(self)
            AFLAG = self.AFLAG;
            REACT = self.REACT;
            DEFL = self.DEFL;
            ELE_FOR = self.ELE_FOR;
            
        end
        
    end
    
    % Private methods go here
    methods (Access = private)
       
        %% Create Node Vector
        function a_nodes = create_nodes(self)
            numnodes = self.nnodes;
            nodecoord = self.coord;
            a_nodes = PCMC_Node.empty(numnodes,0);
            
            for i= 1:numnodes
                coord_v = nodecoord(i,:)'; % vector of coordinates
                a_nodes(i) = PCMC_Node(coord_v,i);
            end
        end
        
        %% Create Element Vector
        function a_elements = create_elements(self,nele,ends,Axx,Ayy,Azz,Iyy,Izz,J,E,v,webdir,w)
            a_elements = PCMC_Element.empty(nele,0);
            
            for i = 1:nele
                nodei = ends(i,1);
                nodej = ends(i,2);
                elem_nodes = [self.a_nodes(nodei), self.a_nodes(nodej)];
                a_elements(i) = PCMC_Element(elem_nodes,Axx(i),Ayy(i),Azz(i),Iyy(i),Izz(i),J(i),E(i),v(i),webdir(i,:),w(i,:)); 
            end
        end
        
        %% Create Global Stiffness Matrix
        function stiffness_matrix = CreateGlobalStiffness(self)
            numele = self.nele;
            numnodes = self.nnodes;
            dof_num = numnodes*6;
            stiffness_matrix = sparse(dof_num,dof_num);
            for i = 1:numele
                ktemp = zeros(numnodes*6);
                elem_stiff = self.a_elements(i).GetGlobalElasticStiffness;                
                elem_dof = self.a_elements(i).GetGlobalDOF;
                ktemp(elem_dof(:),elem_dof(:)) = elem_stiff;
                
                stiffness_matrix = stiffness_matrix + ktemp;
            end
        end
        
        %% Create Global Load Vectors
        function [P_f,P_n,P_s,FEF_f,FEF_n,FEF_s] = CreateGlobalLoad(self,dof_f,dof_n,dof_s)
            numele = self.nele;
            numnodes = self.nnodes;
            dof_num = numnodes*6;
            P_global = sparse(dof_num,1);
            FEF_global = sparse(dof_num,1);
            cnctd = self.concen'; % Transpose of concentrated load matrix
            
            for i = 1:dof_num
                P_global(i,1) = cnctd(i);
            end
            
            for i = 1:numele
                elem_dof = self.a_elements(i).GetGlobalDOF;
                elem_fef = self.a_elements(i).GetGlobalFEF;
                FEF_global(elem_dof,1) = FEF_global(elem_dof,1) + elem_fef;
            end
            
            % Subscript definitions: 
            %%% f = free
            %%% n = known or prescribed
            %%% s = supports
            
            P_f = P_global(dof_f);      % load @ free dof
            P_n = P_global(dof_n);      % load @ kNown dof / prescribed
            P_s = P_global(dof_s);      % load @ support
            FEF_f = FEF_global(dof_f);  % FEF @ free dof
            FEF_n = FEF_global(dof_n);  % FEF @ kNown dof / prescribed
            FEF_s = FEF_global(dof_s);  % FEF @ support
        end
        
        %% Classify DOF
        % Classifies degrees of freedom based on values in fixity matrix.
        function [dof_f,dof_n,dof_s] = ClassifyDOF(self)
            fix_tr = self.fixity_transpose;
            dof_f = find(isnan(fix_tr));
            dof_n = find(fix_tr == 0); 
            dof_s = find(fix_tr ~=0 & ~isnan(fix_tr));
        end
            
        %% Compute Stiffness Submatrices
        % Extracts submatrices from global stiffness matrix.
        function [kff, kfn, knf, knn, ksf, ksn] = ComputeStiffSubMat(self,stiffness_matrix,dof_f,dof_n,dof_s)
            kff = stiffness_matrix(dof_f,dof_f);
            kfn = stiffness_matrix(dof_f,dof_n);
            knf = stiffness_matrix(dof_n,dof_f);
            knn = stiffness_matrix(dof_n,dof_n);
            ksf = stiffness_matrix(dof_s,dof_f);
            ksn = stiffness_matrix(dof_s,dof_n);
        end
        
        %% Check Kff Matrix
        function CheckKFF(self,kff)
            cond_num = condest(kff);
            sigfig = log10(cond_num);
            disp("Condition number of kff matrix: " + cond_num);
            disp("Significant figures lost through calculations: " + sigfig);
            
            % Check if unstable structure, otherwise (else), analysis = successful
            if sigfig > 12 % sigfigs lost is greater than 12, or only 4 sigfigs carried
                self.AFLAG=0;
            else
                self.AFLAG=1;
            end
        end
        
        %% Compute Displacements & Reactions
        function [DEFL, d_n, d_f] = ComputeDispReact(self,kff,kfn,knf,knn,ksf,ksn,dof_f,dof_n,dof_s,P_f,P_n,P_s,FEF_f,FEF_n,FEF_s)
            % DEFL ----- let displacements be denoted by "d"
            d_n = self.fixity_transpose(dof_n);
            d_f = (kff\(P_f - FEF_f - kfn*d_n)); % displacements(delta) at free degree of freedom
            
            DEFL = zeros(self.nnodes,6)';
            DEFL(dof_f) = d_f;
            DEFL(dof_n) = d_n;
            
            % REACT ----- let reactions be denoted by "R"
            R_n = knf*d_f + knn*d_n + FEF_n - P_n;
            R_s = ksf*d_f + ksn*d_n + FEF_s - P_s; 
            
            % Output to MASTAN
            
            DEFL = DEFL';
            self.REACT = DEFL; % Assign as private property
            
            % REACT
            REACT = zeros(self.nnodes,6)';
            REACT(dof_n) = R_n;
            REACT(dof_s) = R_s;
            
            REACT = REACT';
            self.DEFL = REACT; % Assign as private property
        end
        
        % Recover Element Forces
        function [ELE_FOR] = RecoverElemFor(self)
            DEFL = self.REACT;
            DEFL_t = DEFL';
            
            % Initialize vector
            ELE_FOR = zeros(self.nele,12);      
            for i = 1:self.nele
                DOF = GetGlobalDOF(self.a_elements(i));
                d_ele = DEFL_t(DOF);
                ELE_FOR_e = self.a_elements(i).ComputeElemFor(d_ele);
                ELE_FOR(i,:) = ELE_FOR_e';
            end
            self.ELE_FOR = ELE_FOR;
        end
        
        % Compute Error
        function ComputeError(self,kff,kfn,d_f,d_n,P_f,FEF_f,dof_f)
           loadvector = P_f - FEF_f;
           Error = kff*d_f - (loadvector - kfn*d_n);
           disp("Difference between back-calculated and original load vectors is ")
           disp(Error)
        end
        
    end
end