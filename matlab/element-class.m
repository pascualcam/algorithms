classdef PCMC_Element < handle
% Element class for a 3-dimensional framed structure
    
    % Private properties
    properties (Access = private)
        length;                 % Length of element
        elem_nodes;             % Element nodes as PCMC_Node class
        gamma_transformation;   % Transformation matrix of the element
        local_stiffness;        % Local stiffness matrix of the element
        global_stiffness;       % Global stiffness matrix of the element
        global_DOF;             % Global degrees of freedom for the element (12x1)
        local_fef;              % Local Fixed End Forces (FEF) of the element (12x1)
        global_fef;             % Global Fixed End Forces (FEF) of the element (12x1)
    end
    
    % Public methods go here
    methods (Access = public)
        %% Constructor
        %  Replace XYZ by your initials before proceeding
        function self = PCMC_Element(elem_nodes,Axx,Ayy,Azz,Iyy,Izz,J,E,v,webdir,w)
            self.elem_nodes = elem_nodes;
            self.length = self.ComputeLength();
                 
            self.gamma_transformation = self.ComputeTransformationMatrix(webdir);
            self.local_stiffness;
            self.global_stiffness = self.ComputeStiffnessMatrix(Axx,Ayy,Azz,Iyy,Izz,J,E,v);
            
            self.global_DOF = RetrieveDOF(self);
            self.local_fef = ComputeFEF(self,w);
            self.global_fef = self.gamma_transformation'*self.local_fef;
             
        end
        
        %% Getter Functions %%
        %% Get element nodes
        function elem_nodes = GetElementNodes(self)
            elem_nodes = self.elem_nodes;
        end
        
        %% Get element's length
        function length = GetElemLength(self)
            length = self.length;
        end
        
        %% Get Element's Transformation Matrix
        function gamma_transformation = GetTransformationMat(self)
            gamma_transformation = self.gamma_transformation;
        end
        
        %% Get Element's Local Stiffness Matrix
        function local_stiffness = GetLocalStiffness(self)
            local_stiffness = self.local_stiffness;
        end
        
        %% Get Element's Global Stiffness Matrix
        function global_stiffness = GetGlobalElasticStiffness(self)
            global_stiffness = self.gamma_transformation'*self.global_stiffness*self.gamma_transformation;
        end
        
        %% Get Element's Local Fixed End Forces
        % Returns local fixed end forces (FEF) (12x1)
        function local_fef = GetLocalFEF(self)
            local_fef = self.local_fef;
        end
        
        %% Get Element's Global Fixed End Forces
        % Returns global fixed end forces (FEF) (12x1)
        function global_fef = GetGlobalFEF(self)
            global_fef = self.global_fef;
        end
        
        %% Get Element's Global DOF
        % Returns numbered degrees of freedom (12x1)
        function global_DOF = GetGlobalDOF(self)
            global_DOF = self.global_DOF;
        end

        %% Get Element Forces
        function [elem_for] = ComputeElemFor(self,delta_global)
            % Convert global displacements to local coordinates
            % using the transformation matrix
            delta_local = self.gamma_transformation*delta_global;
            
            % Get element forces in local coordinates
            elem_for = self.local_stiffness*delta_local;
            elem_for = elem_for + self.local_fef;
        end
    end
    
    % Private methods go here
    methods (Access = private)
        %% Compute the element's length
        function length = ComputeLength(self)
            node_i = GetNodeCoord(self.elem_nodes(1));
            node_j = GetNodeCoord(self.elem_nodes(2));
            node_diff = node_i - node_j;
            length = norm(node_diff);
        end
        
        %% Compute the element's geometric transformation matrix
        % Coordinate transformation matrix for a 3D member.
        function gamma_transformation = ComputeTransformationMatrix(self,webdir)
            node_1 = self.elem_nodes(1).GetNodeCoord();
            node_2 = self.elem_nodes(2).GetNodeCoord();
            % Find individual coordinates for each node
            xi = node_1(1);
            yi = node_1(2);
            zi = node_1(3);
            xj = node_2(1);
            yj = node_2(2);
            zj = node_2(3);
            % Construct transformation matrix, local
            lambda_x = (xj-xi)/self.length;
            mu_x = (yj-yi)/self.length;
            nu_x = (zj-zi)/self.length;
            xlocal = [lambda_x mu_x nu_x];
            ylocal = webdir;
            zlocal = cross(xlocal,ylocal);
            gamma = [xlocal; ylocal; zlocal];   % Local transformation matrix
            % Assemble global
            gamma_transformation = zeros(12);
            gamma_transformation(1:3,1:3) = gamma;
            gamma_transformation(4:6,4:6) = gamma;
            gamma_transformation(7:9,7:9) = gamma;
            gamma_transformation(10:12,10:12) = gamma;
        end
            
        %% Compute the element's elastic stiffness matrix in local and global coordinates
        function [local_stiffness, global_stiffness] = ComputeStiffnessMatrix(self,Axx,Ayy,Azz,Iyy,Izz,J,E,v)            
            G = E/(2*(1+v));
            phi_y = 12*E*Izz/(G*Ayy*(self.length)^2);
            phi_z = 12*E*Iyy/(G*Azz*(self.length)^2);
            % Create matrix components:
            comp_1 = E*Axx/(self.length);
            comp_2 = 12*E*Izz/(((self.length)^3)*(1+phi_y));
            comp_3 = 6*E*Izz/(((self.length)^2)*(1+phi_y));
            comp_4 = 12*E*Iyy/(((self.length)^3)*(1+phi_z)); 
            comp_5 = 6*E*Iyy/(((self.length)^2)*(1+phi_z));
            comp_6 = G*J/(self.length);
            comp_7 = (4+phi_z)*E*Iyy/((self.length)*(1+phi_z));
            comp_8 = (4+phi_y)*E*Izz/((self.length)*(1+phi_y));
            comp_9 = (2-phi_z)*E*Iyy/((self.length)*(1+phi_z));
            comp_10 = (2-phi_y)*E*Izz/((self.length)*(1+phi_y));
            % Assemble
            quad_1 = [comp_1 0 0 0 0 0;
                    0 comp_2 0 0 0 comp_3;
                    0 0 comp_4 0 -1*comp_5 0;
                    0 0 0 comp_6 0 0;
                    0 0 -1*comp_5 0 comp_7 0;
                    0 comp_3 0 0 0 comp_8];
            quad_4 = [comp_1 0 0 0 0 0;
                    0 comp_2 0 0 0 -1*comp_3;
                    0 0 comp_4 0 comp_5 0;
                    0 0 0 comp_6 0 0;
                    0 0 comp_5 0 comp_7 0;
                    0 -1*comp_3 0 0 0 comp_8];
            quad_2 = [-1*comp_1 0 0 0 0 0;
                    0 -1*comp_2 0 0 0 comp_3;
                    0 0 -1*comp_4 0 -1*comp_5 0;
                    0 0 0 -1*comp_6 0 0;
                    0 0 comp_5 0 comp_9 0;
                    0 -1*comp_3 0 0 0 comp_10];
            quad_3 = [-1*comp_1 0 0 0 0 0;
                    0 -1*comp_2 0 0 0 -1*comp_3;
                    0 0 -1*comp_4 0 comp_5 0;
                    0 0 0 -1*comp_6 0 0;
                    0 0 -1*comp_5 0 comp_9 0;
                    0 comp_3 0 0 0 comp_10];
            % Assemble stiffness matrices
            local_stiffness = [quad_1 quad_2; quad_3 quad_4];
            T = self.gamma_transformation;
            global_stiffness = T'*local_stiffness*T;
            
            self.local_stiffness = local_stiffness;
            self.global_stiffness = global_stiffness;
            
        end
        
        %% Retrieve Element DOF
        % Returns numbered degrees of freedom (12x1)
        function global_dof = RetrieveDOF(self)
            %global_dof = zeros(12,1);
            global_dof = [self.elem_nodes(1).GetNodeDOF; self.elem_nodes(2).GetNodeDOF];
        end
        
         %% Compute Element FEF
         % Returns fixed end forces (FEF) in local coordinates (12x1)
         % Input: w (1x3)
        function [global_fef,local_fef] = ComputeFEF(self,w)
            % Individual components 
            L = ComputeLength(self);
            wx=w(1);
            wy=w(2);
            wz=w(3);
            Vyy=wy*L/2;
            Vzz=wz*L/2;
            Pxx=wx*L/2;
            Mzy=(wy*L^2)/12;
            Myz=(wz*L^2)/12;
            % Assemble 12x1 FEF Matrix
            local_fef = zeros(12,1);
            local_fef(1,1) =-Pxx;
            local_fef(2,1) =-Vyy;
            local_fef(3,1) =-Vzz;
            local_fef(4,1) =0;
            local_fef(5,1) =Myz;
            local_fef(6,1) =-Mzy;
            local_fef(7,1) =-Pxx;
            local_fef(8,1) =-Vyy;
            local_fef(9,1) =-Vzz;
            local_fef(10,1) =0;
            local_fef(11,1) =-Myz;
            local_fef(12,1) =Mzy;
            self.local_fef = local_fef;
            
            global_fef = self.gamma_transformation*local_fef;
            self.global_fef = global_fef;
        end
    end
end
